SUMMARY = "Alexa Automotive Core - Core Module"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://platform/include/AACE/Core/PlatformInterface.h;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

DEPENDS = "sqlite3 avs-device-sdk"

inherit pkgconfig aac-module devlibsonly
EXTRA_OECMAKE += "-DAAC_DEFAULT_LOGGER_ENABLED=${AAC_DEFAULT_LOGGER_ENABLED} \
                  -DAAC_DEFAULT_LOGGER_LEVEL=${AAC_DEFAULT_LOGGER_LEVEL} \
                  -DAAC_DEFAULT_LOGGER_SINK=${AAC_DEFAULT_LOGGER_SINK}"
