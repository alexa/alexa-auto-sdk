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
export VPA_USE_GSTREAMER=${VPA_USE_GSTREAMER:-1}
export VPA_USE_GSTREAMER_V0=${VPA_USE_GSTREAMER_V0:-0}
export VPA_USE_PORTAUDIO=${VPA_USE_PORTAUDIO:-0}
export VPA_USE_WAKEWORD_KTTI=${VPA_USE_WAKEWORD_KTTI:-0}
export VPA_USE_WAKEWORD_SENSORY=${VPA_USE_WAKEWORD_SENSORY:-0}
export VPA_USE_ALEXACOMMS=${VPA_USE_ALEXACOMMS:-0}
export VPA_USE_AMAZONLITE=${VPA_USE_AMAZONLITE:-1}
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
		if [ ! -z $(which chrpath) -a "${lib}" = "curl" ]; then
			chrpath -d ${TARGET_SYSROOT_DIR}/usr/lib/libcurl.so
			do_error_check
		fi
	done

	export TARGET_SYSROOT_DIR=${target_sysroot_dir}
}

vpa_build_avs_sdk() {
	local avs_build_dir=${VPA_BUILD_DIR}/avs/${TARGET_PLATFORM}_${CMAKE_BUILD_TYPE}
	local avs_cmake_options="${VPA_CMAKE_OPTIONS} \
		-DOPUS=ON \
		-DACSDK_DEBUG_LOG_ENABLED=ON \
		-DBUILD_TESTING=${VPA_ENABLE_TESTS} \
		-DBUILD_GMOCK=${VPA_ENABLE_TESTS} \
		-DBUILD_GTEST=${VPA_ENABLE_TESTS} ./AVSCommon/CMakeLists.txt
		-DACSDK_EMIT_SENSITIVE_LOGS=${VPA_USE_SENSITIVE_LOGS} \
		-DACSDK_LATENCY_LOG=${VPA_USE_LATENCY_LOGS}
		-DGSTREAMER_MEDIA_PLAYER=${VPA_USE_GSTREAMER} \
		-DGSTREAMER_VERSION_V0=${VPA_USE_GSTREAMER_V0} \
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
	if [ ${VPA_USE_AMAZONLITE} -eq 1 ]; then
		export VPA_DEPS_LIB_LIST="${VPA_DEPS_LIB_LIST} pryon-lite"
		local model_name="D.en-US.alexa.cpp"
		local model_path=${VPA_TARGET_SYSROOT_DIR}/usr/share/pyron-lite/models
		avs_cmake_options="${avs_cmake_options} \
			-DAMAZONLITE_KEY_WORD_DETECTOR=ON \
			-DAMAZONLITE_KEY_WORD_DETECTOR_LIB_PATH=${VPA_TARGET_SYSROOT_DIR}/usr/lib/libpryon_lite.so \
			-DAMAZONLITE_KEY_WORD_DETECTOR_INCLUDE_DIR=${VPA_TARGET_SYSROOT_DIR}/usr/include \
			-DAMAZONLITE_KEY_WORD_DETECTOR_EMBEDDED_MODEL_CPP_PATH=${model_path}/${model_name}"
	fi

	vpa_build_dependency_libraries

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
	local aac_build_dir=${VPA_BUILD_DIR}/aac/${TARGET_PLATFORM}_${CMAKE_BUILD_TYPE}/modules
	local aac_cmake_options="${VPA_CMAKE_OPTIONS} \
		-DAAC_EMIT_SENSITIVE_LOGS=OFF \
		-DAAC_EMIT_LATENCY_LOGS=ON \
		-DAAC_DEFAULT_LOGGER_ENABLED=ON \
		-DAAC_DEFAULT_LOGGER_LEVEL=Verbose \
		-DAAC_DEFAULT_LOGGER_SINK=Console \
		-DAAC_VERSION=2.0.0 \
		-DAAC_HOME=${CMAKE_INSTALL_PREFIX} \
		-DVPA_SYSROOT_PATH=${VPA_TARGET_SYSROOT_DIR}
		"

	local aac_build_target=(core alexa navigation phone-control contact-uploader cbl address-book vpa)

	if [ ${VPA_USE_AMAZONLITE} -eq 1 ]; then
		aac_build_target+=( 'amazonlite' )
	fi

	for target in "${aac_build_target[@]}"
	do
		echo "###############################################"
		echo "# Start build AAC Module - ${target}"
		echo "###############################################"
		if [ "${target}" = "amazonlite" ]; then
			aac_cmake_options="${aac_cmake_options} \
				-DPATH_TO_WW_LOCALE_MODELS=${VPA_TARGET_SYSROOT_DIR}/usr/share/pryon-lite/models"
		fi
		cmake ${aac_cmake_options} ${AAC_SDK_DIR}/modules/${target}/CMakeLists.txt -B${aac_build_dir}/${target}
		do_error_check
		pushdir ${aac_build_dir}/${target}
		make -j${NCORES} install
		do_error_check
		popdir
	done
}

vpa_build_aac_extension() {
	local aac_ext_src_dir=${AAC_SDK_DIR}/extensions/experimental
	local aac_ext_build_dir=${VPA_BUILD_DIR}/aac/${TARGET_PLATFORM}_${CMAKE_BUILD_TYPE}/extensions/modules
	local aac_cmake_options="${VPA_CMAKE_OPTIONS} \
		-DAAC_EMIT_SENSITIVE_LOGS=OFF \
		-DAAC_EMIT_LATENCY_LOGS=ON \
		-DAAC_DEFAULT_LOGGER_ENABLED=ON \
		-DAAC_DEFAULT_LOGGER_LEVEL=Verbose \
		-DAAC_DEFAULT_LOGGER_SINK=Console \
		-DAAC_VERSION=2.0.0 \
		-DAAC_HOME=${CMAKE_INSTALL_PREFIX} \
		-DGSTREAMER_VERSION_V0=${VPA_USE_GSTREAMER_V0} \
		"

	local aac_build_ext_target=(gstreamer)
	for target in "${aac_build_ext_target[@]}"
	do
		if [ "${target}" = "gstreamer" -a ${VPA_USE_GSTREAMER} -eq 0 ]; then continue; fi
		echo "###############################################"
		echo "# Start build AAC Extension - ${target}"
		echo "###############################################"
		cmake ${aac_cmake_options} ${aac_ext_src_dir}/${target}/modules/${target}/CMakeLists.txt -B${aac_ext_build_dir}/${target}
		do_error_check
		pushdir ${aac_ext_build_dir}/${target}
		make -j${NCORES} install
		do_error_check
		popdir
	done
}

vpa_build_aidaemon() {
	local aidaemon_build_dir=${VPA_BUILD_DIR}/aac/${TARGET_PLATFORM}_${CMAKE_BUILD_TYPE}/sampleapp
	local aidaemon_cmake_options="${VPA_CMAKE_OPTIONS} \
		-DAAC_ENABLE_ADDRESS_SANITIZER=OFF \
		-DAAC_HOME=${CMAKE_INSTALL_PREFIX} \
		-DVPA_SYSROOT_PATH=${VPA_TARGET_SYSROOT_DIR} \
		"

	if [ ${VPA_USE_ALEXACOMMS} -eq 1 ]; then
		aidaemon_cmake_options="${aidaemon_cmake_options} -DALEXACOMMS=ON"
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
	if [ ${VPA_USE_AMAZONLITE} -eq 1 ]; then
		aidaemon_cmake_options="${aidaemon_cmake_options} -DAMAZONLITE=ON"
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

vpa_install_run_script() {
	local vpa_dir=
	local rw_dir=
	local config_dir=
	local output_dir=${VPA_OUTPUT_DIR}/${TARGET_PLATFORM}

	if [ -f ${AAC_SDK_DIR}/VPA/Host/configAIDaemon.json ]; then
		cp -rpa ${AAC_SDK_DIR}/VPA/Host/configAIDaemon.json ${output_dir}
		do_error_check
	fi

	# pclinux
	if [ -z ${CROSS_COMPILE} ]; then
		sed "3i VPA_TOP_DIR=\${PWD}" < ${AAC_SDK_DIR}/VPA/Host/run_vpa.sh.in > ${output_dir}/run_vpa.sh
		do_error_check
		vpa_dir="\${VPA_TOP_DIR}"
		rw_dir=${vpa_dir}
		config_dir=${vpa_dir}
	else # AIVI
		vpa_dir=/var/opt/obigo/obigo_bin/SA
		rw_dir=/var/opt/bosch/dynweb/obigo/obigo_apps/SA/resource
		config_dir=${rw_dir}
		sed "6,18d;23i export DBUS_SESSION_BUS_ADDRESS=\"unix:path=/tmp/shared/iddbus/lxcdbus\"" \
			< ${AAC_SDK_DIR}/VPA/Host/run_vpa.sh.in > ${output_dir}/run_vpa.sh
	fi

	sed -e "s#@vpa_dir@#${vpa_dir}#g;
			s#@rw_dir@#${rw_dir}#g;
			s#@config_dir@#${config_dir}#g" -i ${output_dir}/run_vpa.sh
	do_error_check
	chmod a+x ${output_dir}/run_vpa.sh
	do_error_check
}

vpa_populate_assets() {
	local output_dir=${VPA_OUTPUT_DIR}/${TARGET_PLATFORM}
	local dest_dir=${BUILD_OUTPUT_DIR}/target-${TARGET_PLATFORM}/SA
	local asset_dir=${dest_dir}/opt/AAC/etc

	if [ ! -d ${asset_dir} ]; then mkdir -p ${asset_dir}; fi

	if [ -d ${output_dir}/etc/certs ]; then
		cp -rpa ${output_dir}/etc/certs ${asset_dir}
		do_error_check
	fi

	if [ -f ${AAC_SDK_DIR}/VPA/Host/config.json.in -a -z "${CROSS_COMPILE}" ]; then
		cp -rpa ${AAC_SDK_DIR}/VPA/Host/config.json.in ${asset_dir}
		do_error_check
	fi
	if [ -f ${output_dir}/etc/menu.json ]; then
		cp -rpa ${output_dir}/etc/menu.json ${dest_dir}
		do_error_check
	fi
}

vpa_populate_output() {
	local output_dir=${VPA_OUTPUT_DIR}/${TARGET_PLATFORM}
	local dest_dir=${BUILD_OUTPUT_DIR}/target-${TARGET_PLATFORM}/SA
	local dest_syslib_dir=${dest_dir}/syslibs
	local dest_lib_dir=${dest_dir}/lib
	local dest_bin_dir=${dest_dir}/bin

	mkdir -p ${dest_lib_dir}
	mkdir -p ${dest_syslib_dir}
	mkdir -p ${dest_bin_dir}

	if [ -f ${output_dir}/run_vpa.sh ]; then
		cp -rpa ${output_dir}/run_vpa.sh ${dest_dir}
		do_error_check
	fi

	if [ -f ${output_dir}/configAIDaemon.json ]; then
		cp -rpa ${output_dir}/configAIDaemon.json ${dest_dir}
		do_error_check
	fi

	if [ -f ${output_dir}/bin/SampleApp ]; then
		cp -rpa ${output_dir}/bin/SampleApp ${dest_bin_dir}
		do_error_check
	fi

	if [ -d ${output_dir}/lib ]; then
		if [ $(ls ${output_dir}/lib | wc -l) -gt 0 ]; then
			cp -rpa ${output_dir}/lib/lib*.so* ${dest_lib_dir}
			do_error_check
		fi
	fi

	if [ -d ${VPA_TARGET_SYSROOT_DIR}/usr/lib ]; then
		if [ $(ls ${VPA_TARGET_SYSROOT_DIR}/usr/lib | wc -l) -gt 0 ]; then
			# TODO. temporary fix
			# libAVSCommon.so links libcurl-gnutls.so.4 instead of libcurl.so in SDK not target(AIVI)
			# It is difficult to fix link issue. I should be fixed later
			if [ "${TARGET_PLATFORM}" = "${TARGET_PCLINUX_NAME}" ]; then
				pushdir ${VPA_TARGET_SYSROOT_DIR}/usr/lib
				find . -xtype l -delete
				if [ ! -e libcurl-gnutls.so.4 ]; then
					ln -s libcurl.so libcurl-gnutls.so.4
					do_error_check
				fi
				popdir
			fi
			cp -rpa ${VPA_TARGET_SYSROOT_DIR}/usr/lib/lib*.so* ${dest_syslib_dir}
			do_error_check
		fi
	fi
}

vpa_set_cmake_environments() {
	if [ "${SET_DEBUG}" = "${YES}" ]; then
		export CMAKE_BUILD_TYPE="DEBUG"
	else
		export CMAKE_BUILD_TYPE="RELEASE"
	fi

	if [ ${VPA_USE_GSTREAMER} -eq 1 ]; then
		if [ "${GSTREAMER_VERSION}" = "0.10" ]; then VPA_USE_GSTREAMER_V0=1; fi
	fi

	export CMAKE_INSTALL_PREFIX=${VPA_OUTPUT_DIR}/${TARGET_PLATFORM}
	export VPA_CMAKE_OPTIONS="${COMMON_CMAKE_TOOLCHAIN_OPTIONS} \
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
	vpa_build_avs_sdk
	vpa_build_aac_modules
	vpa_build_aac_extension
	vpa_build_aidaemon

	vpa_install_run_script

	vpa_populate_assets
	vpa_populate_output

	# recovery
	export PKG_CONFIG_PATH=${pkg_config_path}
}

# wrapper
build_service_agent() {
	vpa_main
}
