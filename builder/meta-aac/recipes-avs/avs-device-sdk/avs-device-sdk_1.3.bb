require avs-device-sdk.inc

SRC_URI[md5sum] = "421bad1f844695655a1342687dd63c36"
SRC_URI[sha256sum] = "e3a511d5a7386da43cfdd6bdc91584d784008374647824af85a15799618d069d"

SRC_URI_append_qnx = " file://0001-qnx-Use-int64_t-explicitly.patch"