#!/bin/bash

BUILDER_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_HOME=${BUILDER_HOME}/..

#
# Option
#

usageExit() {
	echo "Usage: $0 [options]"
	echo ""
	echo " -p  Package name"
	echo " -t  Target name"
	echo " -c  Clean"
	echo " -g  Build with debugging options"
	echo ""
	echo " -b  Build directory"
	echo ""
	echo " -d  Run in Docker"
	echo ""
	echo " -a  Android API Level"
	echo ""
	exit 1
}

while getopts ":p:t:cgb:ida:" opt; do
	case $opt in
		p ) PACKAGE="$OPTARG";;
		t ) TARGET="$OPTARG";;
		c ) COMMAND="clean";;
		g ) DEBUG_BUILD="1";;
		b ) BUILD_DIR="$OPTARG";;
		i ) COMMAND="init";;
		d ) MODE="docker";;
		a ) ANDROID_API_LEVEL="$OPTARG";;
		\?) usageExit;;
	esac
done
shift $((OPTIND-1))

# Default values
TARGET=${TARGET:-native}
MODE=${MODE:-oe}
COMMAND=${COMMAND:-build}
DEBUG_BUILD=${DEBUG_BUILD:-0}
BUILD_DIR=$(realpath ${BUILD_DIR:-"${BUILDER_HOME}/build"})
DEPLOY_DIR="${BUILDER_HOME}/deploy"
ANDROID_API_LEVEL=${ANDROID_API_LEVEL:-22}

# External values
: ${MODULES_PATH:="${SDK_HOME}/modules"}
: ${ANDROID_TOOLCHAIN:="${BUILDER_HOME}/android-toolchain"}

#
# Param Checks
#

if [ $(uname) = "Darwin" ] && [ ${MODE} = "oe" ]; then
	echo "NOTE: Builder only runs within Docker with macOS host"
	if [ ${TARGET} = "native" ]; then
		echo "WARNING: \"native\" target will be built for Generic Linux"
	fi
	echo "Switching to Docker mode..."
	MODE="docker"
fi

if [ ${MODE} = "oe" ]; then
	if [ -z ${OE_CORE_PATH} ] || [ ! -d ${OE_CORE_PATH} ]; then
		echo "Invalid OE_CORE_PATH"
		exit 1
	fi
elif [ ${MODE} = "docker" ]; then
	echo "*******************"
	echo "*** Docker Mode ***"
	echo "*******************"
	DOCKER_WORKDIR="/workdir"
	if [ ! -z ${PACKAGE} ]; then
		DOCKER_EXTRA="${DOCKER_EXTRA} -p ${PACKAGE}"
	fi
	if [ ${COMMAND} = "clean" ]; then
		DOCKER_EXTRA="${DOCKER_EXTRA} -c"
	fi
	if [ ${DEBUG_BUILD} = "1" ]; then
		DOCKER_EXTRA="${DOCKER_EXTRA} -g"
	fi
	DOCKER_EXTRA="${DOCKER_EXTRA} -b ${DOCKER_WORKDIR}/build"
	DOCKER_EXTRA="${DOCKER_EXTRA} -t ${TARGET}"
	${BUILDER_HOME}/scripts/run-docker.sh "aac/builder/build.sh" ${DOCKER_EXTRA}
	exit 0
else
	echo "Unsupported Mode ${MODE}"
	exit 1
fi

if [[ ${TARGET} = "android"* ]]; then
	case ${TARGET} in
		androidx86)
			ANDROID_ABI="x86"
			;;
		androidarm)
			ANDROID_ABI="armeabi-v7a"
			;;
	esac
	if [ ! -z ${ANDROID_ABI} ]; then
		# Always run setup script
		${BUILDER_HOME}/scripts/setup-android-toolchain.sh ${ANDROID_ABI} ${ANDROID_API_LEVEL} || \
			(echo "ERROR: Android toolchain setup failed" && exit 1)
	fi
	# Export into BitBake
	export ANDROID_TOOLCHAIN
	export ANDROID_PLATFORM="android-${ANDROID_API_LEVEL}"
fi

if [[ ${TARGET} = "qnx7"* ]]; then
	QNX_HOME="${HOME}/qnx700"
	if [ ! -d ${QNX_HOME} ]; then
		echo "ERROR: QNX 7.0.0 SDP is not installed."
		exit 1
	fi
fi

#
# OE Initialization
#

init_local_conf() {
	BBLAYER_CONF=${BUILD_DIR}/conf/bblayers.conf
	LOCAL_CONF=${BUILD_DIR}/conf/local.conf

	echo "BBLAYERS += \"${BUILDER_HOME}/meta-aac\"" >> ${BBLAYER_CONF}

	# Copy default local.conf
	cp ${BUILDER_HOME}/local.conf.example ${LOCAL_CONF}

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
	echo "BB_HASHBASE_WHITELIST += \"AWS_CONTAINER_CREDENTIALS_RELATIVE_URI AWS_DEFAULT_REGION AWS_REGION\"" >> ${LOCAL_CONF}

	# Register SDK modules
	echo "BBFILES += \"${MODULES_PATH}/*/*.bb\"" >> ${LOCAL_CONF}
}

oe_init() {
	echo "Using BUILD_DIR=${BUILD_DIR}"
	if [ ! -d ${BUILD_DIR} ]; then
		# Init local conf later
		INIT="ON"
	fi

	# Initialize BB with a build directory
	source ${OE_CORE_PATH}/oe-init-build-env ${BUILD_DIR}

	# Export whitelist for ECS
	export BB_ENV_EXTRAWHITE="${BB_ENV_EXTRAWHITE} AWS_CONTAINER_CREDENTIALS_RELATIVE_URI AWS_DEFAULT_REGION AWS_REGION"
	# Export whitelist for additional environments
	export BB_ENV_EXTRAWHITE="${BB_ENV_EXTRAWHITE} DEBUG_BUILD ANDROID_TOOLCHAIN ANDROID_PLATFORM"

	if [ ! -z ${INIT} ]; then
		init_local_conf
	fi
}

execute_bitbake() {
	if [ ${DEBUG_BUILD} = "1" ]; then
		echo "NOTE: Enable debug build"
		export DEBUG_BUILD
	fi
	MACHINE=${TARGET} bitbake $@ ${PACKAGE}
}

copy_images() {
	mkdir -p ${DEPLOY_DIR}
	if [ -d ${DEPLOY_DIR}/${TARGET} ]; then
		rm -rf ${DEPLOY_DIR}/${TARGET}
	fi
	cp -R ${BUILD_DIR}/tmp*/deploy/images/${TARGET} ${DEPLOY_DIR}
}

# Execute command
case ${COMMAND} in
	"init" )
		oe_init
		;;
	"build" )
		oe_init
		if [ -z ${PACKAGE} ]; then
			PACKAGE=aac-image-minimal
		fi
		execute_bitbake && copy_images
		;;
	"clean" )
		oe_init
		if [ -z ${PACKAGE} ]; then
			rm -rf cache downloads sstate-cache tmp* *.log
		else
			execute_bitbake -c cleansstate
		fi
		;;
	* )
		echo "Unknown command: ${COMMAND}"
		exit 1
		;;
esac
