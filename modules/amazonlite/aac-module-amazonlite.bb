SUMMARY = "Alexa Automotive Core - AmazonLite Module"
HOMEPAGE = "http://github.com/alexa"
LICENSE = "CLOSED"

DEPENDS = "avs-device-sdk aac-module-core aac-module-alexa"

inherit aac-module devlibsonly

# Install files for C++ SampleApp execution
EXTRA_OECMAKE += "-DPATH_TO_WW_LOCALE_MODELS=${datadir}/pryon-lite/models"
