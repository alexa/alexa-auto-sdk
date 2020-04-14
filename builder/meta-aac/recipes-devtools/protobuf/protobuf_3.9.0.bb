SUMMARY = "Protocol Buffers - structured data serialisation mechanism"
DESCRIPTION = "Protocol Buffers are a way of encoding structured data in an \
efficient yet extensible format. Google uses Protocol Buffers for almost \
all of its internal RPC protocols and file formats."
HOMEPAGE = "https://github.com/google/protobuf"
SECTION = "console/tools"
LICENSE = "BSD-3-Clause"

PACKAGE_BEFORE_PN = "${PN}-compiler ${PN}-lite"

RDEPENDS_${PN}-compiler = "${PN}"
RDEPENDS_${PN}-dev += "${PN}-compiler"

LIC_FILES_CHKSUM = "file://LICENSE;md5=37b5762e07f0af8c74ce80a8bda4266b"

SRC_URI = "https://github.com/protocolbuffers/protobuf/releases/download/v${PV}/protobuf-cpp-${PV}.tar.gz \
           file://0001-Makefile.am-include-descriptor.cc-when-building-libp.patch \
           file://0001-protobuf-fix-configure-error.patch"
SRC_URI[md5sum] = "5dcc4684b7e91c45fbb9e35622d85387"
SRC_URI[sha256sum] = "12d5facbb9c974be261debb81fa48f4f5be25830d43a64b552edcc4b95cd6cb6"

EXTRA_OECONF += " --with-protoc=echo"

inherit autotools-brokensep

FILES_${PN}-compiler = "${bindir} ${libdir}/libprotoc${SOLIBS}"
FILES_${PN}-lite = "${bindir} ${libdir}/libprotobuf-lite${SOLIBS}"

BBCLASSEXTEND = "native nativesdk"