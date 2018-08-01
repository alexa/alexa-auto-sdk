#!/bin/bash
set -e

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${THISDIR}/common.sh

#
# Option
#

usageExit() {
	echo "Usage: cmake [options] [<module>...]"
	echo ""
	echo " -h,--help                         = Print usage information and exit."
	echo ""
	echo " -t,--enable-tests                 = Enable unit tests."
	echo " -m,--modules-path <path>          = AAC modules path"
	echo " -b,--build-dir <directory>        = Specify a build directory."
	echo ""
	echo " -r,--search-path <path>           = Specify dependencies search root path."
	echo "                                     Equivalent to CMAKE_FIND_ROOT_PATH. (Default: /usr/local)"
	echo " -i,--install-prefix <path>        = Specify installation path."
	echo "                                     Equivalent to CMAKE_INSTALL_PREFIX. (Default: /opt/AAC)"
	echo " -g,--generator <generator-name>   = Specify CMake generator name. Equivalent to G."
	echo "                                     (Default: Unix Makefiles)"
	echo " -x <args>                         = Extra CMake options."
	echo ""
	exit 1
}

PARAMS="$@"
POSITIONAL=()
while [[ $# -gt 0 ]]; do
	key="$1"
	case $key in
		-h|--help)
		usageExit
		shift
		;;
		-r|--search-path)
		AVS_ROOT_PATH="$2"
		shift
		shift
		;;
		-i|--install-prefix)
		INSTALL_PREFIX="$2"
		shift
		shift
		;;
		-g|--generator)
		GENERATOR="$2"
		shift
		shift
		;;
		-m|--module-path)
		MODULES_PATH="$2"
		shift
		shift
		;;
		-t|--enable-tests)
		ENABLE_TESTS="ON"
		shift
		;;
		-x)
		EXTRA_CMAKE_OPTIONS="$2"
		shift
		shift
		;;
		-b|--build-dir)
		BUILD_DIR="$2"
		shift
		shift
		;;
		-*|--*)
		echo "ERROR: Unknown option '$1'"
		usageExit
		shift
		;;
		*)
		POSITIONAL+=("$1")
		shift
		;;
	esac
done
set -- "${POSITIONAL[@]}"

# Default values
AVS_ROOT_PATH=$(realpath ${AVS_ROOT_PATH:-"/usr/local"})
INSTALL_PREFIX=$(realpath ${INSTALL_PREFIX:-"/opt/AAC"})
GENERATOR=${GENERATOR:-"Unix Makefiles"}
MODULES_PATH=$(realpath ${MODULES_PATH:-"${SDK_HOME}/modules"})
BUILD_DIR=$(realpath ${BUILD_DIR:-"${BUILDER_HOME}/cmake"})

MODULES=${@:-"core alexa navigation phone-control"}

BINARY_DIR=${BUILD_DIR}/build

add_project() {
	PROJECT_PATH="${1}"
	NAME="${2}"
	echo "add_subdirectory(${PROJECT_PATH} ${BINARY_DIR}/${NAME})" >> ${BUILD_DIR}/CMakeLists.txt
}

init_cmake_file() {
	mkdir -p ${BUILD_DIR}

	cat > ${BUILD_DIR}/CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(AAC VERSION ${SDK_BASE_VERSION})

EOF

	if [ ! -z ${ENABLE_TESTS} ]; then
		echo "enable_testing()" >> ${BUILD_DIR}/CMakeLists.txt
		echo "set(AAC_ENABLE_TESTS ON)" >> ${BUILD_DIR}/CMakeLists.txt
	fi

	note "Search AAC modules within ${MODULES_PATH}..."
	for module in $MODULES ; do
		MODULE_PATH=${MODULES_PATH}/${module}
		if [ -e ${MODULE_PATH}/CMakeLists.txt ]; then
			note "Module found ${module}"
			add_project ${MODULE_PATH} ${module}
		fi
	done

	# Append further test after adding modules
	if [[ ! -z ${ENABLE_TESTS} && -d ${SDK_HOME}/integration ]]; then
		note "Enable integration tests"
		add_project ${SDK_HOME}/integration integration
	fi

	note "CMake file is ready at ${BUILD_DIR}/CMakeLists.txt"
}

execute_cmake() {
	note "AVS Device SDK root is ${AVS_ROOT_PATH}"
	note "Will install AAC modules into ${INSTALL_PREFIX}"
	note "CMake Generator: ${GENERATOR}"

	# Initialize root CMake file
	init_cmake_file

	# Clean binary dir
	mkdir -p ${BINARY_DIR} && cd ${BINARY_DIR}

	cmake .. \
	-DCMAKE_FIND_ROOT_PATH="${AVS_ROOT_PATH}" \
	-DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
	-G "${GENERATOR}" \
	${EXTRA_CMAKE_OPTIONS}
}

note "Target modules: ${MODULES}"

(execute_cmake && note "CMake is configured at ${BINARY_DIR}") \
	|| rm -rf ${BINARY_DIR}
