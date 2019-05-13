SUMMARY = "Alexa Auto SDK C++ Sample App"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://SampleApp/src/main.cpp;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

DEPENDS = "aac-module-core aac-module-alexa aac-module-navigation aac-module-phone-control aac-module-cbl aac-sample-audio"

PACKAGECONFIG[alexacomms] = "-DALEXACOMMS=ON,,aac-module-communication"
PACKAGECONFIG[amazonlite] = "-DAMAZONLITE=ON,,aac-module-amazonlite"
PACKAGECONFIG[lvc] = "-DLOCALVOICECONTROL=ON,,aac-module-car-control aac-module-local-skill-service aac-module-local-voice-control"

# Use GStreamer by default
AUDIO_PLATFORM ?= "-DGSTREAMER=ON"
# Except QNX and Android
AUDIO_PLATFORM_qnx = "-DFILEAUDIO=ON"
AUDIO_PLATFORM_android = ""

EXTRA_OECMAKE += "${AUDIO_PLATFORM}"

inherit aac-module
