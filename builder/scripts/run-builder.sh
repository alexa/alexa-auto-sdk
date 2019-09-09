#!/bin/bash
set -e

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${THISDIR}/common.sh

#
# Option
#

exit_with_usage() {
	echo "Usage: build.sh [<platform>|clean] [options] [<extension-path> ...]"
	echo ""
	echo " -h,--help                  = Print usage information and exit."
	echo ""
	echo " -t,--target <target-name>  = Specify the target; use comma-separated list for multiple targets. (Default: All targets)"
	echo "                              If not specified, then all possible targets will be attempted to built."
	echo " -g,--debug                 = Build with debugging options."
	echo ""
	echo " --android-api <integer>    = Specify Android API Level. (Default: 22)"
	echo " --qnx7sdp-path <path>      = Specify QNX7 installation directory. (Default: ~/qnx700)"
	echo " --enable-sensitive-logs    = Enable sensitive logs only when building with debugging options."
	echo " --enable-latency-logs      = Enable user perceived latency logs only when building with debugging options."
	echo " --enable-tests             = Enable building test packages for AAC and AVS modules."
	echo ""
	echo " --force-docker             = Force builds to happen inside an Ubuntu docker container."
	echo " --use-mbedtls              = Force using mbedTLS for libcurl."
	echo ""
	echo " --default-logger-enabled <enabled> = Enable/disable the default engine logger: On, Off. (Default: On)"
	echo "                                      If enabled, there must be logger level and sink, either explicitly set or default."
	echo " --default-logger-level <level>     = Set the logger level for the default engine logger: Verbose, Info, Metric, Warn, Error, Critical."
	echo "                                      Default: Info for release builds, Verbose for debug builds."
	echo " --default-logger-sink <sink>       = Set the logger sink for the default engine logger: Console, Syslog."
	echo "                                      Default: Syslog for Android build targets, Console for all other build targets."
	echo ""
	exit 1
}

PLATFORM=${1}
shift

if [ -z ${PLATFORM} ]; then
	error "Please specify the platform"
	exit_with_usage
fi

DEFINES=()
POSITIONAL=()
while [[ $# -gt 0 ]]; do
	key="$1"
	case $key in
		-h|--help)
		exit_with_usage
		shift
		;;
		-t|--target)
		TARGET="$2"
		shift
		shift
		;;
		-g|--debug)
		DEBUG_BUILD="1"
		shift
		;;
		--android-api)
		ANDROID_API_LEVEL="$2"
		shift
		shift
		;;
		--qnx7sdp-path)
		QNX_BASE="$2"
		shift
		shift
		;;
		--enable-sensitive-logs)
		SENSITIVE_LOGS="1"
		shift
		;;
		--enable-latency-logs)
		LATENCY_LOGS="1"
		shift
		;;
		--enable-tests)
		ENABLE_TESTS="1"
		shift
		;;
		--default-logger-enabled)
		DEFAULT_LOGGER_ENABLED="$2"
		shift
		shift
		;;
		--default-logger-level)
		DEFAULT_LOGGER_LEVEL="$2"
		shift
		shift
		;;
		--default-logger-sink)
		DEFAULT_LOGGER_SINK="$2"
		shift
		shift
		;;
		--force-docker)
		FORCE_DOCKER="1"
		shift
		;;
		--use-mbedtls)
		USE_MBEDTLS="1"
		shift
		;;
		-D*=*)
		DEFINES+=("$1")
		shift
		;;
		-*|--*)
		error "Unknown option '$1'"
		exit_with_usage
		shift
		;;
		*) # Any other additional arguments
		POSITIONAL+=("$1")
		shift
		;;
	esac
done
set -- "${POSITIONAL[@]}"

# Default values
TARGET=${TARGET:-""}
DEBUG_BUILD=${DEBUG_BUILD:-0}
ANDROID_API_LEVEL=${ANDROID_API_LEVEL:-22}
QNX_BASE=${QNX_BASE:-"${HOME}/qnx700"}
SENSITIVE_LOGS=${SENSITIVE_LOGS:-0}
LATENCY_LOGS=${LATENCY_LOGS:-0}
ENABLE_TESTS=${ENABLE_TESTS:-0}
FORCE_DOCKER=${FORCE_DOCKER:-0}
USE_MBEDTLS=${USE_MBEDTLS:-0}

SCRIPT_OPTIONS=""
EXTRA_MODULES=$@

# Clean deploy dir
DEPLOY_DIR="${BUILDER_HOME}/deploy"
mkdir -p ${DEPLOY_DIR} && rm -rf ${DEPLOY_DIR}/*
echo "SDK Version: ${SDK_VERSION}" > ${DEPLOY_DIR}/buildinfo.txt

init_extra_local_conf() {
	local extra_local_conf=$1

	touch ${extra_local_conf} && echo > ${extra_local_conf}

	echo "ANDROID_API_LEVEL = \"${ANDROID_API_LEVEL}\"" >> ${extra_local_conf}
	if [ ${DEBUG_BUILD} = "1" ]; then
		note "Enable debug build"
		echo "DEBUG_BUILD = \"1\"" >> ${extra_local_conf}
		if [ ${SENSITIVE_LOGS} = "1" ]; then
			note "Enable sensitive logs"
			echo "AAC_SENSITIVE_LOGS = \"1\"" >> ${extra_local_conf}
		fi
	fi
	if [ ${LATENCY_LOGS} = "1" ]; then
		note "Enable user perceived latency logs"
		echo "AAC_LATENCY_LOGS = \"1\"" >> ${extra_local_conf}
	fi
	if [ ${ENABLE_TESTS} = "1" ]; then
		note "Enable tests to build"
		echo "AAC_ENABLE_TESTS = \"1\"" >> ${extra_local_conf}
		echo "AVS_ENABLE_TESTS = \"1\"" >> ${extra_local_conf}
	fi
	if [ "${DEFAULT_LOGGER_ENABLED}" ]; then
		note "Set default logger enabled: ${DEFAULT_LOGGER_ENABLED}"
		echo "AAC_DEFAULT_LOGGER_ENABLED = \"${DEFAULT_LOGGER_ENABLED}\"" >> ${extra_local_conf}
	fi
	if [ "${DEFAULT_LOGGER_LEVEL}" ]; then
		note "Set default logger level: ${DEFAULT_LOGGER_LEVEL}"
		echo "AAC_DEFAULT_LOGGER_LEVEL = \"${DEFAULT_LOGGER_LEVEL}\"" >> ${extra_local_conf}
	fi
	if [ "${DEFAULT_LOGGER_SINK}" ]; then
		note "Set default logger sink: ${DEFAULT_LOGGER_SINK}"
		echo "AAC_DEFAULT_LOGGER_SINK = \"${DEFAULT_LOGGER_SINK}\"" >> ${extra_local_conf}
	fi
	if [ ${FORCE_DOCKER} = "1" ]; then
		note "Force using Docker for builer"
		SCRIPT_OPTIONS="${SCRIPT_OPTIONS} --force-docker"
	fi
	if [ ${USE_MBEDTLS} = "1" ]; then
		note "Force using mbedTLS for libCURL"
		echo "PACKAGECONFIG_pn-curl = \"mbedtls nghttp2\"" >> ${extra_local_conf}
	fi
}

build_sdk() {
	local oe_target=""
	local extra_local_conf="${BUILDER_HOME}/.extralocal.conf"
	local gstreamer_extension="${SDK_HOME}/extensions/experimental/gstreamer"
	local sample_app="${SDK_HOME}/samples/cpp/aac-sample-cpp.bb"
	local platform=$1

	case ${platform} in
	"android")
		oe_target="androidarm,androidarm64,androidx86,androidx86-64"
		;;
	"qnx7")
		oe_target="qnx7arm64,qnx7x86-64"
		extensions="${sample_app}"
		export QNX_BASE
		;;
	"native")
		oe_target="native"
		extensions="${sample_app} ${gstreamer_extension}"
		;;
	"poky")
		oe_target="pokyarm,pokyarm64"
		extensions="${sample_app} ${gstreamer_extension}"
		;;
	"agl")
		oe_target="aglarm64"
		extensions="${gstreamer_extension}"
		;;
	*)
		error "Unknown platform: ${platform}"
		exit_with_usage
	esac

	# Check if the targets from the command line is valid
	IFS=',' read -ra cmd_targets <<< "${TARGET}"
	for cmd_target in "${cmd_targets[@]}"; do
		if [[ ${cmd_target} != ${platform}* ]]; then
			error "Invalid target: ${cmd_target}"
		fi
	done

	# Use all targets by default
	oe_target=${TARGET:-"${oe_target}"}
	note "Targets: ${oe_target}"

	# Prepare extra bitbake variables
	init_extra_local_conf ${extra_local_conf}

	# Run BitBake
	${THISDIR}/run-bitbake.sh ${SCRIPT_OPTIONS} -t ${oe_target} ${extensions} ${extra_local_conf} ${DEFINES[@]} ${EXTRA_MODULES}
}

if [ ${PLATFORM} = "clean" ]; then
	${THISDIR}/run-bitbake.sh -c
	exit $?
fi

# Run OE build for all platforms
build_sdk ${PLATFORM}

if [ ${PLATFORM} = "android" ]; then
	# Run Gradle build for Android platform
	gradle_options=""
	if [ ${DEBUG_BUILD} = 1 ]; then
		gradle_options="${gradle_options} -g"
	fi
	${THISDIR}/run-gradle.sh ${gradle_options} ${EXTRA_MODULES}
fi
