require avs-device-sdk.inc

# Alexa Auto modifications
# * SpeechEncoder + OPUS support
SRC_URI += "file://0001-SpeechEncoder-Introduce-SpeechEncoder-with-OPUS-supp.patch \
            file://0002-AIP-Only-16kHz-is-supported-for-OPUS-now.patch \
            file://0003-AIP-Initial-SpeechEncoder-support.patch"

# Enable OPUS by default
PACKAGECONFIG += "opus"
PACKAGECONFIG[opus] = "-DOPUS=ON,,libopus"