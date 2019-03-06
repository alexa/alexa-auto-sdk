SUMMARY = "Cross-platform, open-source make system"
HOMEPAGE = "http://www.cmake.org/"
BUGTRACKER = "http://public.kitware.com/Bug/my_view_page.php"
SECTION = "console/utils"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://doc/cmake/Copyright.txt;md5=8d8c7bc32f8797d23f5cf605d9339d2d"

# Use binary distribution
SRC_URI = "https://cmake.org/files/v3.8/cmake-3.8.2-Linux-x86_64.tar.gz"
SRC_URI[md5sum] = "ab02cf61915e1ad15b8523347ad37c46"
SRC_URI[sha256sum] = "33e4851d3219b720f4b64fcf617151168f1bffdf5afad25eb4b7f5f58cee3a08"

inherit native

INSANE_SKIP_${PN} += "already-stripped"

S = "${WORKDIR}/${BP}-Linux-x86_64"
B = "${S}"

do_install() {
	mkdir -p ${D}${bindir}
	cp -R ${B}/bin/* ${D}${bindir}
	mkdir -p ${D}${datadir}
	cp -R ${B}/share/* ${D}${datadir}
}