SUMMARY = "An SDK for commercial device makers to integrate Alexa directly into connected products."
HOMEPAGE = "https://developer.amazon.com/avs/sdk"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=d92e60ee98664c54f68aa515a6169708"

DEPENDS = "curl nghttp2 googletest sqlite3 openssl"

SRC_URI = "https://github.com/alexa/avs-device-sdk/archive/v${PV}.tar.gz"

# Skip checksum verification
BB_STRICT_CHECKSUM = "0"

inherit pkgconfig cmake devlibsonly unittests

EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=${AAC_BUILD_TYPE} \
                  -DACSDK_EMIT_SENSITIVE_LOGS=${AAC_SENSITIVE_LOGS} \
                  -DACSDK_LATENCY_LOG=${AAC_LATENCY_LOGS}"
# Use GStreamer & PortAudio from Homebrew install
EXTRA_OECMAKE_append_darwin = " -DGSTREAMER_MEDIA_PLAYER=ON \
                                -DPORTAUDIO=ON \
                                -DPORTAUDIO_LIB_PATH=/usr/local/lib/libportaudio.a \
                                -DPORTAUDIO_INCLUDE_DIR=/usr/local/include"

# Install AVS artifacts explicitly into AAC_PREFIX dir
EXTRA_OECMAKE_append = " -DCMAKE_INSTALL_PREFIX:PATH=${AAC_PREFIX}"
FILES_${PN} = "${AAC_PREFIX}/*"
FILES_${PN}-test += "${AAC_PREFIX}/lib/libIntegration${SOLIBS}"
SYSROOT_DIRS_append = " ${AAC_PREFIX}"

# WakeWord: KITT.AI
KITTAT_CMAKE_OPTIONS = "\
  -DKITTAI_KEY_WORD_DETECTOR=ON \
  -DKITTAI_KEY_WORD_DETECTOR_LIB_PATH=${STAGING_DIR_HOST}${libdir}/libsnowboy-detect.a \
  -DKITTAI_KEY_WORD_DETECTOR_INCLUDE_DIR=${STAGING_DIR_HOST}${includedir} \
"
PACKAGECONFIG[kittai] = "${KITTAT_CMAKE_OPTIONS},,snowboy"

# WakeWord: Sensory
SENSORY_CMAKE_OPTIONS = "\
  -DSENSORY_KEY_WORD_DETECTOR=ON \
  -DSENSORY_KEY_WORD_DETECTOR_LIB_PATH=${STAGING_DIR_HOST}${libdir}/libsnsr.a \
  -DSENSORY_KEY_WORD_DETECTOR_INCLUDE_DIR=${STAGING_DIR_HOST}${includedir} \
"
PACKAGECONFIG[sensory] = "${SENSORY_CMAKE_OPTIONS},,truly-handsfree"

# Opus encoding (v1.11 or later)
PACKAGECONFIG[opus] = "-DOPUS=ON,,libopus"

do_install_append() {
	mkdir -p ${D}${testbindir}/${PN}
	find ./ -name "*Test" -exec cp {} ${D}${testbindir}/${PN} \;
	if [ -f ${B}/Integration/src/libIntegration${SOLIBS} ]; then
		cp -R ${B}/Integration/src/libIntegration${SOLIBS} ${D}${AAC_PREFIX}/lib
	fi
	mkdir -p ${D}${AAC_PREFIX}/include
	cp -R ${S}/ThirdParty/rapidjson/rapidjson-1.1.0/include/* ${D}${AAC_PREFIX}/include
	# Copy the Mock headers from AVS SDK
	mkdir -p ${D}${AAC_PREFIX}/include/AVSCommon/SDKInterfaces/test
	cp -R ${S}/AVSCommon/SDKInterfaces/test/AVSCommon/SDKInterfaces/* ${D}${AAC_PREFIX}/include/AVSCommon/SDKInterfaces/test
}