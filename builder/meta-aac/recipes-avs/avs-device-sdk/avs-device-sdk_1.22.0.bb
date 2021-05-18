require avs-device-sdk.inc

SRC_URI[md5sum] = "49a804f14ffeaff42c45b231637b9bd4"
SRC_URI[sha256sum] = "0ed5e501700b63a299088ce00210280284dabec3eff82718775130ac16008a21"

# Alexa Auto modifications
SRC_URI += "file://0001-Auto-SDK-Changes-for-v1.22-AVS-SDK.patch"

# Enable OPUS by default
PACKAGECONFIG += "opus"
