require avs-device-sdk.inc

SRC_URI[md5sum] = "e12947d5fa19ea58c141f80304d03509"
SRC_URI[sha256sum] = "73ce215d04f8412bd8fa951751034dabf83fc58971b3ed8356525586a0835cf2"

# Alexa Auto modifications
SRC_URI += "file://0001-Alexa-Auto-changes-for-v1.19.1-AVS-Device-SDK.patch"

# Enable OPUS by default
PACKAGECONFIG += "opus"