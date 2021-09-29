SUMMARY = "C++ JSON library"
HOMEPAGE = "https://github.com/nlohmann/json"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.MIT;md5=f5f7c71504da070bcf4f090205ce1080"

SRC_URI = "https://github.com/nlohmann/json/archive/v${PV}.tar.gz"
SRC_URI[md5sum] = "c8ac4a38e94b0b55698cd34a65474a6e"
SRC_URI[sha256sum] = "e25cf46631f1a3200d109ca33a683b84538411170ee99cd181db8089a2513189"

S = "${WORKDIR}/json-${PV}"

do_install() {
	mkdir -p ${D}${includedir}
	cp -R ${S}/include/* ${D}${includedir}
}