SUMMARY = "Alexa Automotive Core - Audio Sample Implementation"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://include/AACE/Audio/AudioManager.h;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

DEPENDS = "aac-module-alexa"
export AAC_PKG_CONFIG_PROVIDED = "gstreamer-1.0 gstreamer-app-1.0"

EXTRA_OECMAKE += "-DGSTREAMER=ON"

AAC_PKG_CONFIG_PROVIDED_append_agl = " afb-daemon"
EXTRA_OECMAKE_append_agl = " -DAGL_FRAMEWORK=ON"

inherit pkgconfig aac-module devlibsonly