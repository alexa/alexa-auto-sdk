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
	echo " -t,--target <target-name>  = Specify the target; use comma-separated list for multiple targets."
	echo " -g,--debug                 = Build with debugging options."
	echo ""
	echo " --android-api <integer>    = Specify Android API Level. (Default: 22)"
	echo " --qnx7sdp-path <path>      = Specify QNX7 installation directory. (Default: ~/qnx700)"
	echo " --pokysdk-path <path>      = Specify custom Poky installation directory. (Default: /opt/poky/2.6.1)"
	echo " --linaro-prefix <prefix>   = Specify Linaro toolchain path prefix. See the README for the details. (Default: ~/gcc-linaro-7.4.1-2019.02-)"
	echo " --linaro-sysroots <path>   = Specify the directory where the cross sysroots are located. (Default: ~/sysroots)"
	echo " --enable-sensitive-logs    = Enable inclusion of sensitive data in debugging logs. If you enable sensitive logs, you must also build with"
	echo "                              debugging options (--debug)."
	echo "                              IMPORTANT: If you enable sensitive logs, make sure you redact any sensitive data if posting logs publicly."
	echo " --enable-latency-logs      = Enable user perceived latency logs."
	echo " --enable-curl-logs         = Enable logging of upstream/downstream messages exchanged through the AVS Device SDK. Will only enable in"
	echo "                              debug builds. Enabling this option will also enable sensitive logging. The directory to save the curl logs"
	echo "                              must be configured via the AAC config stream:"
	echo '                              { "aace.alexa" : { "avsDeviceSDK" : { "libcurlUtils" : { "streamLogPrefix" : "<PATH_PREFIX>" } } } }'
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
		--linaro-prefix)
		LINARO_TOOLCHAIN_PREFIX="$2"
		shift
		shift
		;;
		--linaro-sysroots)
		LINARO_SYSROOTS="$2"
		shift
		shift
		;;
		--pokysdk-path)
		POKY_SDK="$2"
		shift
		shift
		;;
		--enable-sensitive-logs)
		SENSITIVE_LOGS="1"
		shift
		;;
		--enable-curl-logs)
		CURL_LOGS="1"
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
		--enable-address-sanitizer)
		# This sets a AAC_ENABLE_ADDRESS_SANITIZER for the consuming application. See samples/cpp/SampleApp/CMakeLists.txt for an example.
		ENABLE_ADDRESS_SANITIZER=1
		shift
		;;
		--enable-coverage)
		ENABLE_COVERAGE=1
		shift
		;;
		--aacs-android)
		AACS_ANDROID="1"
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
CURL_LOGS=${CURL_LOGS:-0}
LATENCY_LOGS=${LATENCY_LOGS:-0}
ENABLE_TESTS=${ENABLE_TESTS:-0}
FORCE_DOCKER=${FORCE_DOCKER:-0}
USE_MBEDTLS=${USE_MBEDTLS:-0}
AACS_ANDROID=${AACS_ANDROID:-0}
COMMS=${COMMS:-0}
LVC=${LVC:-0}
AASB=${AASB:-0}

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
		if [ ${CURL_LOGS} = "1" ]; then
			note "Enable curl logs (for AVS SDK)"
			echo "AAC_CURL_LOGS = \"1\"" >> ${extra_local_conf}
		fi
	else
		note "Enable release build"
		if [ ${CURL_LOGS} = "1" ]; then
			error_and_exit "--enable-curl-logs option not supported in non-DEBUG build."
		fi
		if [ ${SENSITIVE_LOGS} = "1" ]; then
			error_and_exit "--enable-sensitive-logs option not supported in non-DEBUG build."
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
	if [ "${ENABLE_COVERAGE}" = "1" ]; then
		note "Enable coverage for AAC modules"
		echo "AAC_ENABLE_COVERAGE = \"1\"" >> ${extra_local_conf}
		SCRIPT_OPTIONS="${SCRIPT_OPTIONS} --enable-coverage"
	fi
	if [ "${ENABLE_ADDRESS_SANITIZER}" = "1" ]; then
		note "Enable address sanitizer cmake variable"
		echo "AAC_ENABLE_ADDRESS_SANITIZER = \"1\"" >> ${extra_local_conf}
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
		note "Force using Docker for builder"
		SCRIPT_OPTIONS="${SCRIPT_OPTIONS} --force-docker"
	fi
	if [ ${USE_MBEDTLS} = "1" ]; then
		note "Force using mbedTLS for libCURL"
		echo "PACKAGECONFIG_pn-curl = \"mbedtls nghttp2\"" >> ${extra_local_conf}
	fi
	if [ "${LINARO_TOOLCHAIN_PREFIX}" ]; then
		echo "LINARO_TOOLCHAIN_PREFIX = \"${LINARO_TOOLCHAIN_PREFIX}\"" >> ${extra_local_conf}
	fi
	if [ "${LINARO_SYSROOTS}" ]; then
		echo "LINARO_SYSROOTS = \"${LINARO_SYSROOTS}\"" >> ${extra_local_conf}
	fi
	if [ "${POKY_SDK}" ]; then
		echo "POKY_SDK = \"${POKY_SDK}\"" >> ${extra_local_conf}
	fi
}

build_sdk() {
	local available_targets=()
	local available_extensions=(comms lvc)
	local extra_local_conf="${BUILDER_HOME}/.extralocal.conf"
	local audio_extension="${SDK_HOME}/extensions/experimental/system-audio"
	local sample_app="${SDK_HOME}/samples/cpp/aac-sample-cpp.bb"
	local aasb_extension="${SDK_HOME}/extensions/aasb"
	local aasb_comms_extension="${SDK_HOME}/extensions/extras/alexacomms/extensions/aasb-comms"
	local aasb_lvc_extension="${SDK_HOME}/extensions/extras/local-voice-control/extensions/aasb-lvc"
	local audio_extension="${SDK_HOME}/extensions/experimental/system-audio"
	local platform=$1

	case ${platform} in
	"android")
		available_targets=("androidarm" "androidarm64" "androidx86" "androidx86-64")
		if [[ "${AACS_ANDROID}" = "1" ]]
		then
			extensions="${aasb_extension}"
			for i in "${available_extensions[@]}"
			do
				local upper_ext=$(echo ${i} | tr '[:lower:]' '[:upper:]')
				if [[ "${!upper_ext}" = 1 ]]
				then
					ext_name=aasb_${i}_extension
					extensions="${extensions} ${!ext_name}"
                                fi
                        done
		elif [[ "${COMMS}" = "1" ]] && [[ "${AASB}" = "1" ]]
		then
			extensions="${aasb_comms_extension}"
		elif [[ "${LVC}" = "1" ]] && [[ "${AASB}" = "1" ]]
		then
			extensions="${aasb_lvc_extension}"
		fi
		;;
	"qnx7")
		available_targets=("qnx7arm64" "qnx7x86-64")
		extensions="${sample_app} ${audio_extension}"
		export QNX_BASE
		;;
	"linux")
		available_targets=("native" "pokyarm" "pokyarm64" "linaroarmel" "linaroarmhf" "linaroarm64")
		extensions="${sample_app} ${audio_extension}"
		;;
	*)
		error "Unknown platform: ${platform}"
		exit_with_usage
	esac

	# Check if the targets from the command line is valid
	IFS=',' read -ra cmd_targets <<< "${TARGET}"
	for cmd_target in "${cmd_targets[@]}"; do
		local invalid="1"
		for valid_target in "${available_targets[@]}"; do
			if [[ ${cmd_target} == ${valid_target} ]]; then
				note "Valid target: ${cmd_target}"
				invalid="0"
				break
			fi
		done
		[ ${invalid} = "1" ] && error_and_exit "Invalid target: ${cmd_target}"
	done

	[ -z "${TARGET}" ] && error_and_exit "Please specify target. Possible targets are: ${available_targets[*]}"

	note "Targets: ${TARGET}"

	# Prepare extra bitbake variables
	init_extra_local_conf ${extra_local_conf}

	# Run BitBake
	${THISDIR}/run-bitbake.sh ${SCRIPT_OPTIONS} -t ${TARGET} ${extensions} ${extra_local_conf} ${DEFINES[@]} ${EXTRA_MODULES}
}

if [ ${PLATFORM} = "clean" ]; then
	${THISDIR}/run-bitbake.sh -c
	exit $?
fi


check_extra_module() {
        local module=$(realpath ${1})
        local module_name=$(basename ${module})
        if [ "${module_name}" = "alexacomms" ];
        then
                COMMS=1
        elif [ "${module_name}" = "aasb" ];
        then
                AASB=1
        elif [ "${module_name}" = "local-voice-control" ];
        then
                LVC=1
        fi
}

for module in ${EXTRA_MODULES} ; do
	check_extra_module ${module}
done

# Run OE build for all platforms
build_sdk ${PLATFORM}

if [ ${PLATFORM} = "android" ]; then
	# Run Gradle build for Android platform
	gradle_options=""
	if [ ${DEBUG_BUILD} = 1 ]; then
		gradle_options="${gradle_options} -g"
	fi
	${THISDIR}/run-gradle.sh ${gradle_options} ${EXTRA_MODULES} ${extensions}
        if [ ${AACS_ANDROID} = "1" ]; then
		${THISDIR}/run-aacs-android.sh ${gradle_options} ${EXTRA_MODULES} 
	fi
fi
