require avs-device-sdk.inc

SRC_URI[md5sum] = "75aff683cbbfd0cd85ef23fbb2e8865a"
SRC_URI[sha256sum] = "f1be94811fd5b41455d3be4a66ec906dbacb6d17774dac3ef70e9728276c1d5a"

# Alexa Auto modifications
SRC_URI += "file://0001-Alexa-Auto-specific-changes-for-1.11-Device-SDK.patch"

# Enable OPUS by default
PACKAGECONFIG += "opus"