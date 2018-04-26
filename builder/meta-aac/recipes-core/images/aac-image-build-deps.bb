SUMMARY = "Alexa Automotive Core minimal target installation image with -dev packages"
LICENSE = "CLOSED"

inherit aac-image

IMAGE_FEATURES += "dev-pkgs"
IMAGE_INSTALL += "\
    avs-device-sdk \
    googletest-staticdev \
    "