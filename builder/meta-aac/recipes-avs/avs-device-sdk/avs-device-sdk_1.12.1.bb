require avs-device-sdk.inc

SRC_URI[md5sum] = "ece2fda85607567dccb3163bf9311efb"
SRC_URI[sha256sum] = "fba17b343c011410584330d8e268d09f449d16d62007b1a797751721a5616368"

# Alexa Auto modifications
# * SpeechEncoder + OPUS support
SRC_URI += "file://0001-Alexa-Auto-specific-changes-for-1.12.1-Device-SDK.patch"

# Enable OPUS by default
PACKAGECONFIG += "opus"
PACKAGECONFIG[opus] = "-DOPUS=ON,,libopus"