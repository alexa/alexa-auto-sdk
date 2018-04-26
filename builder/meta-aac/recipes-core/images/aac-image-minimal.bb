SUMMARY = "Alexa Automotive Core minimal target installation image with -dev packages"
LICENSE = "CLOSED"

inherit aac-image

IMAGE_FEATURES += "dev-pkgs"
IMAGE_INSTALL += "\
    aac-module-core \
    aac-module-alexa \
    aac-module-navigation \
    "