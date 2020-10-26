SUMMARY = "Alexa Automotive Core - AASB Core"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://engine/include/AASB/Engine/Audio/AASBAudioEngineService.h;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

DEPENDS = "aac-module-core aac-module-aasb websocketpp asio nlohmann"

inherit pkgconfig aac-module devlibsonly