require avs-device-sdk.inc

SRC_URI[md5sum] = "86ea42d13fb67fcc7722c156ba061193"
SRC_URI[sha256sum] = "752a2a44c5873c0417b8bb4611fd91b8db18c981d1af34909df306d7cbcaf308"

# Alexa Auto modifications
SRC_URI += "file://0001-Alexa-Auto-changes-for-v1.17-AVS-Device-SDK.patch"

# Enable OPUS by default
PACKAGECONFIG += "opus"