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
export VPA_USE_ALEXACOMMS=${VPA_USE_ALEXACOMMS:-0}
export VPA_USE_AMAZONLITE=${VPA_USE_AMAZONLITE:-0}
export VPA_USE_LVC=${VPA_USE_LVC:-0}
export VPA_USE_DCM=${VPA_USE_DCM:-0}
export VPA_USE_LOOPBACK_DETECTOR=${VPA_USE_LOOPBACK_DETECTOR:-0}
## Dependency Libraries
export VPA_DEPS_LIB_LIST="openssl nghttp2 curl opus"

#################################################################
# Functions
#################################################################
vpa_build_dependency_libraries() {
	local sysroot_exist=1
	local target_sysroot_dir=${TARGET_SYSROOT_DIR}
	export TARGET_SYSROOT_DIR=${VPA_TARGET_SYSROOT_DIR}

	if [ ! -d ${VPA_TARGET_SYSROOT_DIR} ]; then sysroot_exist=0; mkdir -p ${VPA_TARGET_SYSROOT_DIR}; fi
	for lib in ${VPA_DEPS_LIB_LIST}
	do
		export LIBS_CONFIGURED_FILE_NAME=.${lib%/*}-configured
		export LIBS_COMPILED_FILE_NAME=.${lib%/*}-compiled
		export LIBS_INSTALLED_FILE_NAME=.${lib%/*}-installed
		echo "#####################################################"
		echo "${lib}"
		echo "#####################################################"
		if [ ${sysroot_exist} -eq 0 ]; then rm -f ${SYSLIBS_DEST_DIR}/${lib}*/${LIBS_INSTALLED_FILE_NAME}; fi
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

	echo "###############################################"
	echo "# Start build AVS SDK"
	echo "###############################################"
	# generate Makefile
	cmake ${avs_cmake_options} ${AVS_SDK_DIR}/CMakeLists.txt -B${avs_build_dir}
	do_error_check

	# compile and install
	pushdir ${avs_build_dir}
	make -j${NCORES} install
	do_error_check
	popdir
}

vpa_build_aac_modules() {
	local aac_build_dir=${VPA_BUILD_DIR}/aac/${TARGET_PLATFORM}_${CMAKE_BUILD_TYPE}
	local aac_cmake_options="${CMAKE_OPTIONS} \
		-DAAC_EMIT_SENSITIVE_LOGS=OFF \
		-DAAC_EMIT_LATENCY_LOGS=ON \
		-DAAC_DEFAULT_LOGGER_ENABLED=ON \
		-DAAC_DEFAULT_LOGGER_LEVEL=Verbose \
		-DAAC_DEFAULT_LOGGER_SINK=Console \
		-DAAC_VERSION=2.0.0 \
		-DAAC_HOME=${CMAKE_INSTALL_PREFIX} \
		"
	local aac_build_target=(core alexa navigation phone-control contact-uploader cbl address-book vpa)
	for target in "${aac_build_target[@]}"
	do
		echo "###############################################"
		echo "# Start build AAC Module - ${target}"
		echo "###############################################"
		cmake ${aac_cmake_options} ${AAC_SDK_DIR}/modules/${target}/CMakeLists.txt -B${aac_build_dir}/modules/${target}
		do_error_check
		pushdir ${aac_build_dir}/modules/${target}
		make -j${NCORES} install
		do_error_check
		popdir
	done
}

vpa_build_aac_extension() {
	echo "extension"
}

vpa_build_aidaemon() {
	local aidaemon_build_dir=${VPA_BUILD_DIR}/aac/${TARGET_PLATFORM}_${CMAKE_BUILD_TYPE}/sampleapp
	local aidaemon_cmake_options="${CMAKE_OPTIONS} \
		-DAAC_ENABLE_ADDRESS_SANITIZER=OFF \
		-DAAC_HOME=${CMAKE_INSTALL_PREFIX} \
		"

	if [ ${VPA_USE_ALEXACOMMS} -eq 1 ]; then
		aidaemon_cmake_options="${aidaemon_cmake_options} -DALEXACOMMS=ON"
	fi
	if [ ${VPA_USE_AMAZONLITE} -eq 1 ]; then
		aidaemon_cmake_options="${aidaemon_cmake_options} -DAMAZONLITE=ON"
	fi
	if [ ${VPA_USE_LVC} -eq 1 ]; then
		aidaemon_cmake_options="${aidaemon_cmake_options} -DLOCALVOICECONTROL=ON"
	fi
	if [ ${VPA_USE_GSTREAMER} -eq 1 ]; then
		aidaemon_cmake_options="${aidaemon_cmake_options} -DGSTREAMER=ON"
	fi
	if [ ${VPA_USE_DCM} -eq 1 ]; then
		aidaemon_cmake_options="${aidaemon_cmake_options} -DDCM=ON"
	fi
	if [ ${VPA_USE_LOOPBACK_DETECTOR} -eq 1 ]; then
		aidaemon_cmake_options="${aidaemon_cmake_options} -DLOOPBACK_DETECTOR=ON"
	fi

	echo "###############################################"
	echo "# Start build AIDaemon"
	echo "###############################################"
	cmake ${aidaemon_cmake_options} ${AAC_SDK_DIR}/samples/cpp/SampleApp/CMakeLists.txt -B${aidaemon_build_dir}
	do_error_check
	pushdir ${aidaemon_build_dir}
	make -j${NCORES} install
	do_error_check
	popdir
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
		-DUSE_OBIGO_BUILD_SYSTEM=ON \
		-DOBIGO_AIDAEMON=ON"
}

vpa_main() {
	# backup
	local pkg_config_path=${PKG_CONFIG_PATH}
	local pkg_config_sysroot_dir=${PKG_CONFIG_SYSROOT_DIR}

	# export
	export VPA_TARGET_SYSROOT_DIR=${VPA_OUTPUT_DIR}/${TARGET_PLATFORM}/sysroot
	export PKG_CONFIG_PATH=${VPA_TARGET_SYSROOT_DIR}/usr/lib/pkgconfig:${PKG_CONFIG_PATH}

	vpa_set_cmake_environments
	vpa_build_dependency_libraries
	vpa_build_avs_sdk
	vpa_build_aac_modules
	vpa_build_aac_extension
	vpa_build_aidaemon

	# recovery
	export PKG_CONFIG_PATH=${pkg_config_path}
}

# wrapper
build_service_agent() {
	vpa_main
}
