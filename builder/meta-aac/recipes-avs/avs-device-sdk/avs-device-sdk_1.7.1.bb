require avs-device-sdk.inc

SRC_URI[md5sum] = "b22068f2a4b45ced72bd6f9f628ed52b"
SRC_URI[sha256sum] = "33f830c59398efb6cf0779a701af7acd1157b3244e11de0fd868738069b6ef59"

SRC_URI_append_qnx = " file://0001-qnx-prevent-segmentation-fault.patch"