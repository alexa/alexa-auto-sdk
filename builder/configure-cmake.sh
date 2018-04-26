#!/bin/bash
set -e

BUILDER_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_HOME=${BUILDER_HOME}/..

#
# Option
#

usageExit() {
	echo "Usage: $0 [options] [modules]"
	echo ""
	echo " -r  Dependencies search root path"
	echo " -i  Installation path"
	echo ""
	echo " -g  Generator name"
	echo " -m  AAC modules path"
	echo " -t  Enable unit tests"
	echo ""
	echo " -x  Extra CMake options"
	echo " -b  Build directory"
	echo ""
	exit 1
}

while getopts ":r:i:g:m:tx:b:" opt; do
	case $opt in
		r ) AVS_ROOT_PATH="$OPTARG";;
		i ) INSTALL_PREFIX="$OPTARG";;
		g ) GENERATOR="$OPTARG";;
		m ) MODULES_PATH="$OPTARG";;
		t ) ENABLE_TESTS="ON";;
		x ) EXTRA_CMAKE_OPTIONS="$OPTARG";;
		b ) BUILD_DIR="$OPTARG";;
		\?) usageExit;;
	esac
done
shift $((OPTIND-1))

# Default values
AVS_ROOT_PATH=$(realpath ${AVS_ROOT_PATH:-"/usr/local"})
INSTALL_PREFIX=$(realpath ${INSTALL_PREFIX:-"/opt/AAC"})
GENERATOR=${GENERATOR:-"Unix Makefiles"}
MODULES_PATH=$(realpath ${MODULES_PATH:-"${SDK_HOME}/modules"})
BUILD_DIR=$(realpath ${BUILD_DIR:-"${BUILDER_HOME}/build-cmake"})

MODULES=${@:-"core alexa navigation"}

echo "Target modules: ${MODULES}"

add_project() {
	PROJECT_PATH="${1}"
	NAME="${2}"
	echo "add_subdirectory(${PROJECT_PATH} ${BUILD_DIR}/${NAME})" >> ${BUILD_DIR}/CMakeLists.txt
}

init_cmake_file() {
	cat > ${BUILD_DIR}/CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(AACE)

EOF

	if [ ! -z ${ENABLE_TESTS} ]; then
		echo "enable_testing()" >> ${BUILD_DIR}/CMakeLists.txt
	fi

	echo "Search AAC modules within ${MODULES_PATH}..."
	for module in $MODULES ; do
		MODULE_PATH=${MODULES_PATH}/${module}
		if [ -e ${MODULE_PATH}/CMakeLists.txt ]; then
			echo "Module found ${module}"
			add_project ${MODULE_PATH} ${module}
		fi
	done

	# Append further test after adding modules
	if [[ ! -z ${ENABLE_TESTS} && -d ${SDK_HOME}/tests ]]; then
		echo "Enable integration tests"
		add_project ${SDK_HOME}/tests/integration integration
	fi
}

if [ ! -d ${BUILD_DIR} ]; then
	mkdir -p ${BUILD_DIR}
	init_cmake_file
fi

echo "AVS Device SDK root is ${AVS_ROOT_PATH}"
echo "Will install AAC modules into ${INSTALL_PREFIX}"
echo "CMake Generator: ${GENERATOR}"

mkdir -p ${BUILD_DIR}/tmp && cd ${BUILD_DIR}/tmp
cmake .. \
-DCMAKE_FIND_ROOT_PATH="${AVS_ROOT_PATH}" \
-DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
-G "${GENERATOR}" \
${EXTRA_CMAKE_OPTIONS}

echo "CMake is configured at ${BUILD_DIR}/tmp"