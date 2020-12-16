SUMMARY = "Alexa Automotive Core - AASB Text to Speech"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://engine/include/AASB/Engine/TextToSpeech/AASBTextToSpeechEngineService.h;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

DEPENDS = "aac-module-text-to-speech aac-module-aasb aac-aasb-core nlohmann"

inherit pkgconfig aac-module devlibsonly
