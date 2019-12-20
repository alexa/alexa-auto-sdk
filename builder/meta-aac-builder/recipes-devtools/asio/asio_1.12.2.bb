SUMMARY = "C++ library for network and low-level I/O programming"
HOMEPAGE = "https://sourceforge.net/projects/asio"
LICENSE = "Other"
LIC_FILES_CHKSUM = "file://LICENSE_1_0.txt;md5=e4224ccaecb14d942c71d31bef20d78c"

SRC_URI = "https://sourceforge.net/projects/asio/files/asio/1.12.2%20%28Stable%29/asio-1.12.2.tar.gz/download;downloadfilename=asio-1.12.2.tar.gz"
SRC_URI[md5sum] = "43c53b8d255edc385dc4e704e59b3641"
SRC_URI[sha256sum] = "da5c9fd2726165dc85a4fd3cb86ccd2059ffaa332675aa861262f5b1923ce30e"

S = "${WORKDIR}/asio-${PV}"

do_install() {
	mkdir -p ${D}${includedir}
	cp -R ${S}/include/* ${D}${includedir}
}