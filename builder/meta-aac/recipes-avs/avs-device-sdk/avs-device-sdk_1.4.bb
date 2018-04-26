require avs-device-sdk.inc

SRC_URI[md5sum] = "01802262519fbc0ba837f24b641f1672"
SRC_URI[sha256sum] = "d450b94ae034dd11589e3c0a99621fa41a372661318d5ca3ec7cdb2513748361"

SRC_URI_append_qnx = " file://0001-qnx-Use-int64_t-explicitly.patch"