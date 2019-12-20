SUMMARY = "C++ JSON library"
HOMEPAGE = "https://github.com/Tencent/rapidjson"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://license.txt;md5=ba04aa8f65de1396a7e59d1d746c2125"

SRC_URI = "https://github.com/Tencent/rapidjson/archive/v${PV}.tar.gz"
SRC_URI[md5sum] = "badd12c511e081fec6c89c43a7027bce"
SRC_URI[sha256sum] = "bf7ced29704a1e696fbccf2a2b4ea068e7774fa37f6d7dd4039d0787f8bed98e"

S = "${WORKDIR}/${PN}-${PV}"

do_install() {
	mkdir -p ${D}${includedir}
	cp -R ${S}/include/* ${D}${includedir}
}