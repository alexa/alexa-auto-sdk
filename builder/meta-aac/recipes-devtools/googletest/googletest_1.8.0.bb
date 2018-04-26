SUMMARY = "Google's C++ test framework"
HOMEPAGE = "https://github.com/google/googletest"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://googletest/LICENSE;beginline=4;md5=084c52530c7c2446140fec0f6516355c"

SRC_URI = "https://github.com/google/googletest/archive/release-${PV}.tar.gz"
SRC_URI[md5sum] = "16877098823401d1bf2ed7891d7dce36"
SRC_URI[sha256sum] = "58a6f4277ca2bc8565222b3bbd58a177609e9c488e8a72649359ba51450db7d8"

S = "${WORKDIR}/${PN}-release-${PV}"

RDEPENDS_${PN}-dev = ""
RDEPENDS_${PN}-staticdev = ""

inherit cmake

EXTRA_OECMAKE += "-DBUILD_GTEST=ON \
                  -DBUILD_GMOCK=ON"

do_install() {
	mkdir -p ${D}${libdir}
	cp ${B}/googlemock/gtest/libgtest.a ${D}${libdir}
	cp ${B}/googlemock/gtest/libgtest_main.a ${D}${libdir}
	mkdir -p ${D}${includedir}
	cp -R ${S}/googletest/include/* ${D}${includedir}
	cp ${B}/googlemock/libgmock.a ${D}${libdir}
	cp ${B}/googlemock/libgmock_main.a ${D}${libdir}
	cp -R ${S}/googlemock/include/* ${D}${includedir}
}