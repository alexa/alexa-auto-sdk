#!/bin/bash
set -e

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${THISDIR}/common.sh

#
# Option
#

usageExit() {
	echo "Usage: cmake [options] [cmake-options]"
	echo ""
	echo " -h,--help                         = Print usage information and exit."
	echo ""
	echo " -m,--modules <module-names>       = Specify modules (comma separated) to include."
	echo " -b,--working-dir <directory>      = Specify a working directory."
	echo ""
	echo " -t,--enable-tests                 = Enable unit tests."
	echo " -c,--coverage                     = Build with coverage enabled."
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
		-m|--modules)
		MODULES="$2"
		shift
		shift
		;;
		-b|--build-dir)
		BUILD_DIR="$2"
		shift
		shift
		;;
		-t|--enable-tests)
		ENABLE_TESTS="ON"
		shift
		;;
		-c|--coverage)
		COVERAGE="ON"
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
MODULES=${MODULES:-"core,alexa,navigation,phone-control"}
BUILD_DIR=$(realpath ${BUILD_DIR:-"${BUILDER_HOME}/cmake"})

# Default external values
: ${MODULES_PATH:="${SDK_HOME}/modules"}

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
set(SDK_VERSION ${SDK_VERSION})

EOF

	if [ ! -z ${ENABLE_TESTS} ]; then
		echo "enable_testing()" >> ${BUILD_DIR}/CMakeLists.txt
		echo "set(AAC_ENABLE_TESTS ON)" >> ${BUILD_DIR}/CMakeLists.txt
	fi

	# Build with coverage if requested
	if [[ ! -z ${COVERAGE} ]]; then
		note "Enable code coverage"
		echo 'set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")' >> ${BUILD_DIR}/CMakeLists.txt
		echo 'set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)' >> ${BUILD_DIR}/CMakeLists.txt
	fi

	note "Search AAC modules within ${MODULES_PATH}..."
	# Parse into array
	IFS=',' read -ra modules_array <<< "${MODULES}"
	for module in ${modules_array[@]} ; do
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
	# Initialize root CMake file
	init_cmake_file
	# Clean binary dir
	mkdir -p ${BINARY_DIR} && rm -rf ${BINARY_DIR}/* && cd ${BINARY_DIR}
	# Execute CMake
	cmake .. $@ && note "Build directory is configured at ${BINARY_DIR}"
}

note "Target modules: ${MODULES}"

execute_cmake $@
