SUMMARY = "C++ JSON library"
HOMEPAGE = "https://github.com/nlohmann/json"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.MIT;md5=f5f7c71504da070bcf4f090205ce1080"

SRC_URI = "https://github.com/nlohmann/json/archive/v${PV}.tar.gz"
SRC_URI[md5sum] = "c53592d55e7fec787cf0a406d36098a3"
SRC_URI[sha256sum] = "80c45b090e40bf3d7a7f2a6e9f36206d3ff710acfa8d8cc1f8c763bb3075e22e"

S = "${WORKDIR}/json-${PV}"

do_install() {
	mkdir -p ${D}${includedir}
	cp -R ${S}/include/* ${D}${includedir}
}