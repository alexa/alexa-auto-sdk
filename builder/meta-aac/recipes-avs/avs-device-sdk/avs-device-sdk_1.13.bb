require avs-device-sdk.inc

SRC_URI[md5sum] = "e6d559b87dac64c4426ae0046f4b90cd"
SRC_URI[sha256sum] = "cc103422471d1b436f15081c52960540f11d3278409495e7b2d2ffd44bff10a2"

# Alexa Auto modifications
SRC_URI += "file://0001-Alexa-Auto-specific-changes-for-1.13-Device-SDK.patch"

# Enable OPUS by default
PACKAGECONFIG += "opus"