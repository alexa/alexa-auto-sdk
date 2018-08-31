require avs-device-sdk.inc

SRC_URI[md5sum] = "44b78b8de6a9ec3d15141fd45d6bbcca"
SRC_URI[sha256sum] = "737a3dce09e9401f31c5f1ce84089e1340f79ef6f393d91000d44755ef12b6dc"

# Alexa Auto modifications
# * SpeechEncoder + OPUS support
SRC_URI += "file://0001-SpeechEncoder-Introduce-SpeechEncoder-with-OPUS-supp.patch \
            file://0002-AIP-Only-16kHz-is-supported-for-OPUS-now.patch \
            file://0003-AIP-Initial-SpeechEncoder-support.patch"

# Enable OPUS by default
PACKAGECONFIG += "opus"
PACKAGECONFIG[opus] = "-DOPUS=ON,,libopus"