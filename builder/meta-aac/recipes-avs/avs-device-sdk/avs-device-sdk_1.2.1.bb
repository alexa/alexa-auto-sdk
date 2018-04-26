require avs-device-sdk.inc

SRC_URI[md5sum] = "87d38f2525401e49c0cee80263b49ecd"
SRC_URI[sha256sum] = "18e7f2197d974e69462f7bc0a3e35f59b126b4538dbead9eb31dfd22a234829c"

SRC_URI_append_qnx = " file://0001-qnx-Use-int64_t-explicitly.patch"