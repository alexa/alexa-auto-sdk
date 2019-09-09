SUMMARY = "Alexa Auto SDK installation package with -dev packages"
LICENSE = "Apache-2.0"

inherit aac-image

IMAGE_FEATURES += "dev-pkgs"
IMAGE_INSTALL += "\
    aac-module-core \
    aac-module-alexa \
    aac-module-navigation \
    aac-module-phone-control \
    aac-module-contact-uploader \
    aac-module-cbl \
    aac-module-address-book \
    "