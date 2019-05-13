SUMMARY = "Alexa Automotive Core - Audio Sample Implementation"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://include/AACE/Audio/AudioManager.h;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

DEPENDS = "aac-module-alexa"
export AAC_PKG_CONFIG_PROVIDED = "gstreamer-1.0 gstreamer-app-1.0"

# Use GStreamer by default
AUDIO_PLATFORM ?= "-DGSTREAMER=ON"
# Except QNX and Android
AUDIO_PLATFORM_qnx = "-DFILEAUDIO=ON"
AUDIO_PLATFORM_android = ""

EXTRA_OECMAKE += "${AUDIO_PLATFORM}"

# Enable 4A support for AGL
PACKAGECONFIG[agl] = "-DAGL_FRAMEWORK=ON,,af-binder"
PACKAGECONFIG_append_agl = " agl"

inherit pkgconfig aac-module devlibsonly