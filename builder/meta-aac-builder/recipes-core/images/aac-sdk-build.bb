SUMMARY = "Alexa Auto SDK installation package with -dev packages"
LICENSE = "Apache-2.0"

inherit aac-image

IMAGE_FEATURES += "dev-pkgs"
IMAGE_INSTALL += "\
    aac-module-address-book \
    aac-module-alexa \
    aac-module-apl \
    aac-module-car-control \
    aac-module-cbl \
    aac-module-connectivity \
    aac-module-core \
    aac-module-messaging \
    aac-module-navigation \
    aac-module-phone-control \
    aac-module-text-to-speech \
    aac-module-text-to-speech-provider \
    "