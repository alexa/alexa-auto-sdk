SUMMARY = "Alexa Automotive Core target installation image with -test packages"
LICENSE = "Apache-2.0"

inherit aac-image

IMAGE_FEATURES += "dev-pkgs"
IMAGE_INSTALL += "\
    aac-module-core \
    aac-module-alexa \
    aac-module-navigation \
    aac-module-phone-control \
    aac-module-contact-uploader \
    avs-device-sdk-test \
    aac-module-core-test \
    aac-integration \
    "