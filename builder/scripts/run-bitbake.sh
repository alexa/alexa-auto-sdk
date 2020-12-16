#!/bin/bash

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${THISDIR}/common.sh
source ${THISDIR}/agreement.sh

#
# Option
#

exit_with_usage() {
	echo "Usage: bitbake [options] [<module-path>|<meta-layer>|<extra-bb-file>|<extra-conf-file> ...]"
	echo ""
	echo " -h,--help                  = Print usage information and exit."
	echo ""
	echo " -t,--target <target-name>  = Specify the target; use comma-separated list for multiple targets. (Default: native)"
	echo " -b,--build-dir <directory> = Specify a build directory."
	echo " -c,--clean                 = Clean everything."
	echo ""
	echo " --package <recipe-name>    = Specify OE recipe name explicitly."
	echo " --force-docker             = Force builds to happen inside an Ubuntu docker container."
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
		exit_with_usage
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
		-c|--clean)
		CLEAN="1"
		shift
		;;
		--package)
		PACKAGE="$2"
		shift
		shift
		;;
		--force-docker)
		FORCE_DOCKER="1"
		shift
		;;
		--enable-coverage)
		ENABLE_COVERAGE=1
		EXTRA_CONFS+=("-DAAC_ENABLE_COVERAGE")
		shift
		;;
		-D*=*)
		EXTRA_CONFS+=("${1:2}")
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
TARGET=${TARGET:-native}
CLEAN=${CLEAN:-0}
BUILD_DIR=${BUILD_DIR:-"${BUILDER_HOME}/build"}

EXTRA_MODULES=$@

# Default external values
: ${MODULES_PATH:="${SDK_HOME}/modules"}
: ${ANDROID_TOOLCHAIN:="${BUILDER_HOME}/android-toolchain"}
: ${QNX_BASE:="${HOME}/qnx700"}

#
# Param Checks
#

if [ $(uname) = "Darwin" ] || ([ ! -f /.dockerenv ] && [ "${FORCE_DOCKER}" = "1" ]); then
	if [ $(uname) = "Darwin" ]; then
		note "Builder only runs within Docker with macOS host"
	fi
	if [[ ${TARGET} = *"native"* ]]; then
		warn "\"native\" target will be built for Generic Linux"
	fi
	if [[ ${TARGET} = *"qnx7"* ]]; then
		note "Using QNX 7.0.0 SDP installation path: ${QNX_BASE}"
		# Export into Docker script
		export QNX_BASE
	fi
	note "Switching to Docker mode..."
	${THISDIR}/run-docker.sh "aac/builder/build.sh bitbake" ${PARAMS}
	exit $?
elif [[ $(uname -a) == *"Ubuntu"* ]]; then
	note "Ubuntu host is detected, optimizing for Ubuntu machine"
	HOST_SUPPORT_LAYER="${BUILDER_HOME}/meta-aac-ubuntu"
fi

# Parse into array
IFS=',' read -ra TARGETS <<< "${TARGET}"

if [ -z ${OE_CORE_PATH} ] || [ ! -d ${OE_CORE_PATH} ]; then
	error_and_exit "Invalid OE_CORE_PATH: ${OE_CORE_PATH}"
fi

# Use absolute path
OE_CORE_PATH=$(realpath ${OE_CORE_PATH})
BUILD_DIR=$(realpath ${BUILD_DIR})

# Deploy dir
DEPLOY_DIR="${BUILDER_HOME}/deploy"

# Download repository
DL_DIR="${BUILDER_HOME}/downloads"

#
# Docker Overrides
#

if [ -f /.dockerenv ] && [ -z ${CODEBUILD_SRC_DIR} ]; then
	# Assuming we are running inside the Docker
	DOCKER_WORKDIR="/workdir"
	# We know the Docker image is Ubuntu based
	HOST_SUPPORT_LAYER="${BUILDER_HOME}/meta-aac-ubuntu"
	# Override build directory
	BUILD_DIR="${DOCKER_WORKDIR}/build"
	# Override download directory
	DL_DIR="${DOCKER_WORKDIR}/downloads"
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
	# Export into Bitbake and setup script
	export ANDROID_TOOLCHAIN
	# Always run setup script
	${BUILDER_HOME}/scripts/setup-android-toolchain.sh || \
		error_and_exit "Android toolchain setup failed"
}

#
# QNX Platform Checks
#

qnx_checks() {
	if [ ! -d ${QNX_BASE}/host/linux ]; then
		error_and_exit "Check your QNX 7.0.0 SDP installation and make sure Linux host tools are installed."
	fi
	# Export into Bitbake
	export QNX_BASE
}

#
# OE Initialization
#

EXTRA_BBFILES="${MODULES_PATH}/*/*.bb"
EXTRA_BBLAYERS=""
EXTRA_INSTALLS=""

add_extra_module() {
	local module=$(realpath ${1})
	local module_name=$(basename ${module})
	local sub_module=${2}
	if [[ -d ${module} ]]; then
		if [[ ${module_name} == "meta-"* && -e ${module}/conf/layer.conf ]]; then
			note "Adding meta layer: ${module}"
			EXTRA_BBLAYERS="${EXTRA_BBLAYERS} ${module}"
		elif [[ ! ${sub_module} ]]; then
			note "Search for modules & meta layers: ${module}"
			for content in $(find ${module} -mindepth 1 -maxdepth 1) ; do
				add_extra_module ${content} 1
			done
		elif [[ ${sub_module} && ${module_name} == "modules" ]]; then
			note "Search for modules: ${module}"
			for bbfile in $(find ${module}/*/*.bb) ; do
				add_extra_module ${bbfile} 1
			done
		fi
	elif [[ -e ${module} ]]; then
		if [[ ${module} == *".conf" ]]; then
			note "Adding extra conf file: ${module}"
			echo "# Extra Config: ${module}" >> ${LOCAL_CONF}
			cat ${module} >> ${LOCAL_CONF}
			echo "" >> ${LOCAL_CONF}
			echo "#####" >> ${LOCAL_CONF}
		elif [[ ${module} == *".bb" ]]; then
			note "Adding single module: ${module}"
			EXTRA_BBFILES="${EXTRA_BBFILES} ${module}"
			EXTRA_INSTALLS="${EXTRA_INSTALLS} ${module_name%".bb"}"
		elif [[ ! ${sub_module} ]]; then
			warn "Unknown file: ${module}"
		fi
	else
		warn "Unknown file/directory: ${module}"
	fi
}

clean_lockfile() {
	if [ -e ${BUILD_DIR}/bitbake.lock ]; then
		warn "A build already appears to be in progress, or else a previous build was interrupted."
		read -p "Do you want to proceed? (y/n): " -n 1 -r
		echo
		if [[ $REPLY =~ ^[Yy]$ ]]; then
			note "Removing ${BUILD_DIR}/bitbake.lock..."
			rm -rf ${BUILD_DIR}/bitbake.lock
		else
			exit 1
		fi
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
  ${HOST_SUPPORT_LAYER} \\
  ${OE_CORE_PATH}/meta \\
  ${BUILDER_HOME}/meta-aac \\
  ${BUILDER_HOME}/meta-aac-builder \\
  "
EOF

	# Copy default local.conf
	cp ${BUILDER_HOME}/meta-aac-builder/conf/local.conf.sample ${LOCAL_CONF}

	# Find extra bb files & layers
	for module in ${EXTRA_MODULES} ; do
		add_extra_module ${module}
	done

	# Register extra modules
	echo "BBFILES += \"${EXTRA_BBFILES}\"" >> ${BBLAYER_CONF}
	echo "BBLAYERS += \"${EXTRA_BBLAYERS}\"" >> ${BBLAYER_CONF}

	if [ ! -z ${CODEBUILD_SRC_DIR} ]; then
		# Cache points for AWS CodeBuild
		echo "DL_DIR = \"/root/downloads\"" >> ${LOCAL_CONF}
		echo "SSTATE_DIR = \"/root/sstate-cache\"" >> ${LOCAL_CONF}
	else
		echo "DL_DIR = \"${DL_DIR}\"" >> ${LOCAL_CONF}
	fi

	# Export allow list for ECS
	echo "export AWS_CONTAINER_CREDENTIALS_RELATIVE_URI" >> ${LOCAL_CONF}
	echo "export AWS_DEFAULT_REGION" >> ${LOCAL_CONF}
	echo "export AWS_REGION" >> ${LOCAL_CONF}

	# Avoid rebuilding sstate
	echo "BB_HASHBASE_WHITELIST += \"AWS_CONTAINER_CREDENTIALS_RELATIVE_URI AWS_DEFAULT_REGION AWS_REGION ANDROID_TOOLCHAIN QNX_BASE\"" >> ${LOCAL_CONF}

	# Extra image installation
	if [ ! -z "${EXTRA_INSTALLS}" ]; then
		echo "IMAGE_INSTALL_append = \"${EXTRA_INSTALLS}\"" >> ${LOCAL_CONF}
	fi

	# Extra properties from CLI
	for extra_conf in "${EXTRA_CONFS[@]}"; do
		echo "${extra_conf%=*} = \"${extra_conf#*=}\"" >> ${LOCAL_CONF}
	done
}

execute_bitbake() {
	note "Using BUILD_DIR=${BUILD_DIR}"

	# Set template
	rm -f ${BUILD_DIR}/conf/templateconf.cfg
	export TEMPLATECONF=${BUILDER_HOME}/meta-aac-builder/conf

	# Initialize BB with a build directory
	source ${OE_CORE_PATH}/oe-init-build-env ${BUILD_DIR}

	# Export allow list for ECS
	export BB_ENV_EXTRAWHITE="${BB_ENV_EXTRAWHITE} AWS_CONTAINER_CREDENTIALS_RELATIVE_URI AWS_DEFAULT_REGION AWS_REGION"
	# Export allow list for additional environments
	export BB_ENV_EXTRAWHITE="${BB_ENV_EXTRAWHITE} ANDROID_TOOLCHAIN QNX_BASE"

	MACHINE=${TARGET} bitbake $@ ${PACKAGE}
}

copy_images() {
	mkdir -p ${DEPLOY_DIR}/${TARGET}
	cp -P ${BUILD_DIR}/tmp*/deploy/images/${TARGET}/${PACKAGE}* ${DEPLOY_DIR}/${TARGET} 2>/dev/null || :
	if [ "${ENABLE_COVERAGE}" = "1" ]; then
		note "Copying coverage metadata files to ${DEPLOY_DIR}/${TARGET}/coverage/"
		mkdir -p ${DEPLOY_DIR}/${TARGET}/coverage
		find ${BUILD_DIR}/tmp*/ -name "*.gcno" | xargs cp -ft ${DEPLOY_DIR}/${TARGET}/coverage
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
		exit $?
	fi

	if [ -z ${PACKAGE} ]; then
		PACKAGE=aac-sdk-build
	fi
	execute_bitbake
	exit_if_failure

	copy_images
}

note "SDK Version: ${SDK_VERSION}"
agreement_check

clean_lockfile

# Initialize local configs
init_local_conf

for TARGET in "${TARGETS[@]}"; do
	build
done
