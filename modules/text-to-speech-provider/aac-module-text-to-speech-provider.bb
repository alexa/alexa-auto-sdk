SUMMARY = "Alexa Automotive Core - Text to Speech Provider Module"
HOMEPAGE = "http://github.com/alexa"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://engine/include/AACE/Engine/TextToSpeechProvider/TextToSpeechProviderEngine.h;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

DEPENDS = "avs-device-sdk aac-module-core aac-module-alexa aac-module-text-to-speech nlohmann"

inherit aac-module devlibsonly
