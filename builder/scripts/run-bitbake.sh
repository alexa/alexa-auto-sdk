#!/bin/bash

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${THISDIR}/common.sh

#
# Option
#

usageExit() {
	echo "Usage: oe [options] [<bbrecipe-file>|<meta-layer>|<extra-conf-file>]"
	echo ""
	echo " -h,--help                  = Print usage information and exit."
	echo ""
	echo " -t,--target <target-name>  = Specify the target. (Default: native)"
	echo " -b,--build-dir <directory> = Specify a build directory."
	echo " -g,--debug                 = Build with debugging options."
	echo " -c,--clean                 = Clean eveything."
	echo ""
	echo " --android-api <integer>    = Specify Android API Level. (Default: 22)"
	echo " --qnx7sdp-path <path>      = Specify QNX7 installation directory. (Default: ~/qnx700)"
	echo " --package <recipe-name>    = Specify OE recipe name explicitly."
	echo " --enable-sensitive-logs    = Enable sensitive logs only when building with debugging options."
	echo " --enable-latency-logs      = Enable user perceived latency logs only when building with debugging options."
	echo ""
	exit 1
}

PARAMS="$@"
POSITIONAL=()
EXTRA_CONFS=()
while [[ $# -gt 0 ]]; do
	key="$1"
	case $key in
		-h|--help)
		usageExit
		shift
		;;
		-t|--target)
		TARGET="$2"
		shift
		shift
		;;
		-b|--build-dir)
		BUILD_DIR="$2"
		shift
		shift
		;;
		-g|--debug)
		DEBUG_BUILD="1"
		shift
		;;
		-c|--clean)
		CLEAN="1"
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
		--package)
		PACKAGE="$2"
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
		-D*=*)
		EXTRA_CONFS+=("${1:2}")
		shift
		;;
		-*|--*)
		echo "ERROR: Unknown option '$1'"
		usageExit
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
TARGET=${TARGET:-native}
CLEAN=${CLEAN:-0}
DEBUG_BUILD=${DEBUG_BUILD:-0}
BUILD_DIR=${BUILD_DIR:-"${BUILDER_HOME}/build"}
ANDROID_API_LEVEL=${ANDROID_API_LEVEL:-22}
QNX_BASE=${QNX_BASE:-"${HOME}/qnx700"}
SENSITIVE_LOGS=${SENSITIVE_LOGS:-0}
LATENCY_LOGS=${LATENCY_LOGS:-0}

EXTRA_MODULES=$@

# Default external values
: ${MODULES_PATH:="${SDK_HOME}/modules"}
: ${ANDROID_TOOLCHAIN:="${BUILDER_HOME}/android-toolchain"}

#
# Param Checks
#

if [ $(uname) = "Darwin" ]; then
	note "Builder only runs within Docker with macOS host"
	if [[ ${TARGET} = *"native"* ]]; then
		warn "\"native\" target will be built for Generic Linux"
	fi
	if [[ ${TARGET} = *"qnx7"* ]]; then
		note "Using QNX 7.0.0 SDP installation path: ${QNX_BASE}"
		# Export into Docker script
		export QNX_BASE
	fi
	note "Switching to Docker mode..."
	echo ""
	echo "*******************"
	echo "*** Docker Mode ***"
	echo "*******************"
	echo ""
	${THISDIR}/run-docker.sh "aac/builder/build.sh oe" ${PARAMS}
	exit 0
fi

# Parse into array
IFS=',' read -ra TARGETS <<< "${TARGET}"

if [ -z ${OE_CORE_PATH} ] || [ ! -d ${OE_CORE_PATH} ]; then
	error "Invalid OE_CORE_PATH: ${OE_CORE_PATH}"
fi

# Use absolute path
OE_CORE_PATH=$(realpath ${OE_CORE_PATH})
BUILD_DIR=$(realpath ${BUILD_DIR})

# Clean deploy dir
DEPLOY_DIR="${BUILDER_HOME}/deploy"
mkdir -p ${DEPLOY_DIR} && rm -rf ${DEPLOY_DIR}/*

#
# Docker Overrides
#

if [ -f /.dockerenv ] && [ -z ${CODEBUILD_SRC_DIR} ]; then
	# Assuming we are running inside the Docker
	DOCKER_WORKDIR="/workdir"
	# Override build directory
	BUILD_DIR="${DOCKER_WORKDIR}/build"
	# Override QNX SDP path
	QNX_BASE="${HOME}/qnx700"
	# Override extra modules path
	NORM=""
	for module in ${EXTRA_MODULES} ; do
		path="${module}"
		if [[ ! ${path} =~ ^/ ]]; then
			path="${HOST_PWD}/${path}"
		fi
		# Normalize into relative path from HOST_SDK_HOME
		path="$(python3 -c "import os;print(os.path.relpath(\"${path}\", \"${HOST_SDK_HOME}\"))")"
		# Concat with our SDK_HOME
		path="${SDK_HOME}/${path}"
		if [ ! -e ${path} ]; then
			warn "Skipping invalid module path \"${module}\". The file/directory must be present under SDK directory."
		else
			NORM="${NORM} ${path}"
		fi
	done
	EXTRA_MODULES=${NORM}
fi

#
# Android Platform Checks
#

android_checks() {
	note "Android toolchains will be installed: ${ANDROID_TOOLCHAIN}"
	case ${TARGET} in
		androidx86)
			ANDROID_ABI="x86"
			;;
		androidx86-64)
			ANDROID_ABI="x86_64"
			;;
		androidarm)
			ANDROID_ABI="armeabi-v7a"
			;;
		androidarm64)
			ANDROID_ABI="arm64-v8a"
			;;
		*)
			error "Unsupported Android target: ${TARGET}"
	esac
	# Export into Bitbake and setup script
	export ANDROID_TOOLCHAIN
	# Always run setup script
	${BUILDER_HOME}/scripts/setup-android-toolchain.sh ${ANDROID_ABI} ${ANDROID_API_LEVEL} || \
		error "Android toolchain setup failed"
}

#
# QNX Platform Checks
#

qnx_checks() {
	if [ ! -d ${QNX_BASE}/host/linux ]; then
		error "Check your QNX 7.0.0 SDP installation and make sure Linux host tools are installed."
	fi
	# Export into Bitbake
	export QNX_BASE
}

#
# OE Initialization
#

EXTRA_BBFILES="${MODULES_PATH}/*/*.bb"
EXTRA_BBLAYERS=""

add_extra_bbfile() {
	module=$(realpath ${1})
	sub_module=${2}
	if [[ -d ${module} ]]; then
		if [[ $(basename ${module}) == "meta-"* && -e ${module}/conf/layer.conf ]]; then
			note "Adding meta layer: ${module}"
			EXTRA_BBLAYERS="${EXTRA_BBLAYERS} ${module}"
		elif [[ ! ${sub_module} ]]; then
			note "Search for sub modules: ${module}"
			for sub_module in $(find ${module} -mindepth 1 -maxdepth 1 -type d) ; do
				add_extra_bbfile ${sub_module} 1
			done
		elif [[ ${sub_module} && $(basename ${module}) == "modules" ]]; then
			note "Adding modules: ${module}"
			EXTRA_BBFILES="${EXTRA_BBFILES} ${module}/*/*.bb"
		fi
	elif [[ -e ${module} ]]; then
		if [[ ${module} == *".bb" ]]; then
			note "Adding BB file: ${module}"
			EXTRA_BBFILES="${EXTRA_BBFILES} ${module}"
		elif [[ ${module} == *".conf" ]]; then
			note "Adding extra conf file: ${module}"
			echo "# Extra Config: ${module}" >> ${LOCAL_CONF}
			cat ${module} >> ${LOCAL_CONF}
			echo "" >> ${LOCAL_CONF}
			echo "#####" >> ${LOCAL_CONF}
		else
			warn "Unknown file: ${module}"
		fi
	else
		warn "Unknown file/directory: ${module}"
	fi
}

init_local_conf() {
	mkdir -p ${BUILD_DIR}/conf

	BBLAYER_CONF=${BUILD_DIR}/conf/bblayers.conf
	LOCAL_CONF=${BUILD_DIR}/conf/local.conf

	cat > ${BBLAYER_CONF} <<EOF
LCONF_VERSION = "7"
BBPATH = "\${TOPDIR}"
BBLAYERS = " \\
  ${OE_CORE_PATH}/meta \\
  ${BUILDER_HOME}/meta-aac \\
  "
EOF

	# Copy default local.conf
	cp ${BUILDER_HOME}/meta-aac/conf/local.conf.sample ${LOCAL_CONF}

	# Find extra bb files & layers
	for module in ${EXTRA_MODULES} ; do
		add_extra_bbfile ${module}
	done

	# Register extra modules
	echo "BBFILES += \"${EXTRA_BBFILES}\"" >> ${BBLAYER_CONF}
	echo "BBLAYERS += \"${EXTRA_BBLAYERS}\"" >> ${BBLAYER_CONF}

	if [ ! -z ${CODEBUILD_SRC_DIR} ]; then
		# Cache points for AWS CodeBuild
		echo "DL_DIR = \"/root/downloads\"" >> ${LOCAL_CONF}
		echo "SSTATE_DIR = \"/root/sstate-cache\"" >> ${LOCAL_CONF}
	fi

	# Export white list for ECS
	echo "export AWS_CONTAINER_CREDENTIALS_RELATIVE_URI" >> ${LOCAL_CONF}
	echo "export AWS_DEFAULT_REGION" >> ${LOCAL_CONF}
	echo "export AWS_REGION" >> ${LOCAL_CONF}

	# Avoid rebuilding sstate
	echo "BB_HASHBASE_WHITELIST += \"AWS_CONTAINER_CREDENTIALS_RELATIVE_URI AWS_DEFAULT_REGION AWS_REGION ANDROID_TOOLCHAIN QNX_BASE\"" >> ${LOCAL_CONF}

	# Additional properties
	echo "ANDROID_PLATFORM = \"android-${ANDROID_API_LEVEL}\"" >> ${LOCAL_CONF}
	if [ ${DEBUG_BUILD} = "1" ]; then
		note "Enable debug build"
		echo "DEBUG_BUILD = \"1\"" >> ${LOCAL_CONF}
		if [ ${SENSITIVE_LOGS} = "1" ]; then
			note "Enable sensitive logs"
			echo "AAC_SENSITIVE_LOGS = \"1\"" >> ${LOCAL_CONF}
		fi
	fi
	if [ ${LATENCY_LOGS} = "1" ]; then
		note "Enable user perceived latency logs"
		echo "AAC_LATENCY_LOGS = \"1\"" >> ${LOCAL_CONF}
	fi

	# Extra properties from CLI
	for extra_conf in "${EXTRA_CONFS[@]}"; do
		echo "${extra_conf%=*} = \"${extra_conf#*=}\"" >> ${LOCAL_CONF}
	done

	# AAC version
	echo "DISTRO_VERSION = \"${SDK_BASE_VERSION}\"" >> ${LOCAL_CONF}
	echo "SDK_VERSION = \"${SDK_VERSION}\"" >> ${LOCAL_CONF}
}

execute_bitbake() {
	note "Using BUILD_DIR=${BUILD_DIR}"

	# Set template
	rm -f ${BUILD_DIR}/conf/templateconf.cfg
	export TEMPLATECONF=${BUILDER_HOME}/meta-aac/conf

	# Initialize BB with a build directory
	source ${OE_CORE_PATH}/oe-init-build-env ${BUILD_DIR}

	# Export whitelist for ECS
	export BB_ENV_EXTRAWHITE="${BB_ENV_EXTRAWHITE} AWS_CONTAINER_CREDENTIALS_RELATIVE_URI AWS_DEFAULT_REGION AWS_REGION"
	# Export whitelist for additional environments
	export BB_ENV_EXTRAWHITE="${BB_ENV_EXTRAWHITE} ANDROID_TOOLCHAIN QNX_BASE"

	MACHINE=${TARGET} bitbake $@ ${PACKAGE}
}

copy_images() {
	mkdir -p ${DEPLOY_DIR}/${TARGET}
	cp -P ${BUILD_DIR}/tmp*/deploy/images/${TARGET}/${PACKAGE}* ${DEPLOY_DIR}/${TARGET}
	echo "SDK Version: ${SDK_VERSION}" > ${DEPLOY_DIR}/buildinfo.txt
	if [ -d ${SDK_HOME}/.repo ] && [ $(command -v repo) ]; then
		echo "Repo Info:" >> ${DEPLOY_DIR}/buildinfo.txt
		repo forall -c "echo \${REPO_PROJECT} =\> \${REPO_PATH}: \${REPO_LREV} >> ${DEPLOY_DIR}/buildinfo.txt"
	fi
}

build() {
	note "Start building for ${TARGET}..."
	if [[ ${TARGET} = "android"* ]]; then
		android_checks
	fi
	if [[ ${TARGET} = "qnx"* ]]; then
		qnx_checks
	fi
	if [ ${CLEAN} = "1" ]; then
		if [ -z ${PACKAGE} ] && [ -d ${BUILD_DIR} ]; then
			warn "Attempt to remove entire build directory."
			read -p "Are you sure? (y/n): " -n 1 -r
			echo
			if [[ $REPLY =~ ^[Yy]$ ]]; then
				note "Removing ${BUILD_DIR}..."
				rm -rf ${BUILD_DIR}
			fi
		else
			note "Cleaning cache files for ${PACKAGE}"
			execute_bitbake -c cleansstate
		fi
		exit 0
	fi

	if [ -z ${PACKAGE} ]; then
		PACKAGE=aac-image-minimal
	fi
	execute_bitbake && copy_images
}

note "SDK Version: ${SDK_VERSION}"

# Initialize local configs
init_local_conf

for TARGET in "${TARGETS[@]}"; do
	build
done