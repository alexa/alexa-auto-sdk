SUMMARY = "Alexa Automotive Core - System Audio Module"
HOMEPAGE = "http://github.com/alexa"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://engine/src/SystemAudioEngineService.cpp;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

DEPENDS = "aac-module-core"

inherit pkgconfig aac-module devlibsonly
