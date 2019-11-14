#!/bin/bash

#################################################################
# Environments Variables
#################################################################
## Directories
export AAC_SDK_DIR=${BUILD_TOP_DIR}/serviceagent/aac-sdk
export AVS_SDK_DIR=${BUILD_TOP_DIR}/serviceagent/avs-sdk
export VPA_OUTPUT_DIR=${BUILD_TOP_DIR}/serviceagent/out
export VPA_BUILD_DIR=${BUILD_TOP_DIR}/serviceagent/build
export VPA_TARGET_SYSROOT_DIR=

## Features
### Logger level : ( Verbose | Info | Metric | Warn | Error | Critical )
export VPA_LOGGER_LEVEL=Verbose
### Logger sink : ( Console | Syslog )
export VPA_LOGGER_SINK=Console
export VPA_LOGGER_ENABLED=${VPA_LOGGER_ENABLED:-1}
export VPA_ENABLE_TESTS=${VPA_ENABLE_TESTS:-0}
export VPA_USE_SENSITIVE_LOGS=${VPA_USE_SENSITIVE_LOGS:-0}
export VPA_USE_LATENCY_LOGS=${VPA_USE_LATENCY_LOGS:-1}
export VPA_USE_GSTREAMER=${VPA_USE_GSTREAMER:-0}
export VPA_USE_PORTAUDIO=${VPA_USE_PORTAUDIO:-0}
export VPA_USE_WAKEWORD_KTTI=${VPA_USE_WAKEWORD_KTTI:-0}
export VPA_USE_WAKEWORD_SENSORY=${VPA_USE_WAKEWORD_SENSORY:-0}

## Dependency Libraries
export VPA_DEPS_LIB_LIST="openssl nghttp2 curl opus"

#################################################################
# Functions
#################################################################
vpa_build_dependency_libraries() {
	local target_sysroot_dir=${TARGET_SYSROOT_DIR}
	export TARGET_SYSROOT_DIR=${VPA_TARGET_SYSROOT_DIR}

	if [ ! -d ${VPA_TARGET_SYSROOT_DIR} ]; then mkdir -p ${VPA_TARGET_SYSROOT_DIR}; fi
	for lib in ${VPA_DEPS_LIB_LIST}
	do
		. ${SYSLIBS_BUILD_SCRIPT_DIR}/${lib}.sh
		do_build_${lib}
	done

	export TARGET_SYSROOT_DIR=${target_sysroot_dir}
}

vpa_build_avs_sdk() {
	local avs_build_dir=${VPA_BUILD_DIR}/avs/${TARGET_PLATFORM}_${CMAKE_BUILD_TYPE}
	local avs_cmake_options="${CMAKE_OPTIONS} \
		-DOPUS=ON \
		-DACSDK_DEBUG_LOG_ENABLED=ON \
		-DBUILD_TESTING=${VPA_ENABLE_TESTS} \
		-DBUILD_GMOCK=${VPA_ENABLE_TESTS} \
		-DBUILD_GTEST=${VPA_ENABLE_TESTS} ./AVSCommon/CMakeLists.txt
		-DACSDK_EMIT_SENSITIVE_LOGS=${VPA_USE_SENSITIVE_LOGS} \
		-DACSDK_LATENCY_LOG=${VPA_USE_LATENCY_LOGS}
		-DGSTREAMER_MEDIA_PLAYER=${VPA_USE_GSTREAMER} \
		-DVPA_SYSROOT_PATH=${VPA_TARGET_SYSROOT_DIR}"

	if [ ${VPA_USE_PORTAUDIO} -eq 1 ]; then
		export VPA_DEPS_LIB_LIST="${VPA_DEPS_LIB_LIST} portaudio"
		avs_cmake_options="${avs_cmake_options} \
			-DPORTAUDIO=ON \
			-DPORTAUDIO_LIB_PATH=${VPA_TARGET_SYSROOT_DIR}/usr/lib/libportaudio.a \
			-DPORTAUDIO_INCLUDE_DIR=${VPA_TARGET_SYSROOT_DIR}/usr/include"
	fi
	if [ ${VPA_USE_WAKEWORD_KTTI} -eq 1 ]; then
		export VPA_DEPS_LIB_LIST="${VPA_DEPS_LIB_LIST} snowboy"
		avs_cmake_options="${avs_cmake_options} \
			-DKITTAI_KEY_WORD_DETECTOR=ON \
			-DKITTAI_KEY_WORD_DETECTOR_LIB_PATH=${VPA_TARGET_SYSROOT_DIR}/usr/lib/libsnowboy-detect.a \
			-DKITTAI_KEY_WORD_DETECTOR_INCLUDE_DIR=${VPA_TARGET_SYSROOT_DIR}/usr/include"
	fi
	if [ ${VPA_USE_WAKEWORD_SENSORY} -eq 1 ]; then
		export VPA_DEPS_LIB_LIST="${VPA_DEPS_LIB_LIST} sensory"
		avs_cmake_options="${avs_cmake_options} \
			-DSENSORY_KEY_WORD_DETECTOR=ON \
			-DSENSORY_KEY_WORD_DETECTOR_LIB_PATH=${VPA_TARGET_SYSROOT_DIR}/usr/lib/libsnsr.a \
			-DSENSORY_KEY_WORD_DETECTOR_INCLUDE_DIR=${VPA_TARGET_SYSROOT_DIR}/usr/include"
	fi

	# generate Makefile
	cmake ${avs_cmake_options} ${AVS_SDK_DIR}/CMakeLists.txt -B${avs_build_dir}
	do_error_check

	# compile and install
	pushdir ${avs_build_dir}
	make -j${NCORES} install
	do_error_check
	popdir
}

vpa_build_aac() {
	local aac_cmake_options="${CMAKE_OPTIONS} \
		-DAAC_EMIT_SENSITIVE_LOGS=ON \
		-DAAC_EMIT_LATENCY_LOGS=ON \
		-DAAC_DEFAULT_LOGGER_ENABLED=ON \
		-DAAC_DEFAULT_LOGGER_LEVEL=Verbose \
		-DAAC_DEFAULT_LOGGER_SINK=Console \
		-DAAC_VERSION=2.0.0 \
		"
}

vpa_set_cmake_environments() {
	if [ "${SET_DEBUG}" = "${YES}" ]; then
		export CMAKE_BUILD_TYPE="DEBUG"
	else
		export CMAKE_BUILD_TYPE="RELEASE"
	fi

	export CMAKE_INSTALL_PREFIX=${VPA_OUTPUT_DIR}/${TARGET_PLATFORM}
	export CMAKE_OPTIONS="${COMMON_CMAKE_TOOLCHAIN_OPTIONS} \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
		-DCMAKE_NO_SYSTEM_FROM_IMPORTED=1 \
		-DCMAKE_INSTALL_SO_NO_EXE=0 \
		-DOBIGO_AIDAEMON=ON"
}

vpa_main() {
	# backup
	local pkg_config_path=${PKG_CONFIG_PATH}
	local pkg_config_sysroot_dir=${PKG_CONFIG_SYSROOT_DIR}

	# export
	export VPA_TARGET_SYSROOT_DIR=${VPA_OUTPUT_DIR}/${TARGET_PLATFORM}/sysroot
	export PKG_CONFIG_PATH=${VPA_TARGET_SYSROOT_DIR}/usr/lib/pkgconfig:${PKG_CONFIG_PATH}
	export PKG_CONFIG_SYSROOT_DIR=${VPA_TARGET_SYSROOT_DIR}

	vpa_set_cmake_environments
	vpa_build_dependency_libraries
	vpa_build_avs_sdk
	kill -SIGINT $$

	# recovery
	export PKG_CONFIG_PATH=${pkg_config_path}
	export PKG_CONFIG_SYSROOT_DIR=${pkg_config_sysroot_dir}
}

# wrapper
build_service_agent() {
	vpa_main
}
