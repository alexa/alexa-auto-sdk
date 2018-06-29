SUMMARY = "Alexa Automotive Core minimal target installation image with -dev packages"
LICENSE = "Apache-2.0"

inherit aac-image

IMAGE_FEATURES += "dev-pkgs"
IMAGE_INSTALL += "\
    avs-device-sdk \
    avs-device-sdk-test \
    googletest-staticdev \
    "