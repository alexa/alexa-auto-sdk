SUMMARY = "Cross-platform, open-source make system"
HOMEPAGE = "http://www.cmake.org/"
BUGTRACKER = "http://public.kitware.com/Bug/my_view_page.php"
SECTION = "console/utils"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://doc/cmake/Copyright.txt;md5=f61f5f859bc5ddba2b050eb10335e013"

# Use binary distribution
SRC_URI = "https://cmake.org/files/v3.12/cmake-3.12.3-Linux-x86_64.tar.gz"
SRC_URI[md5sum] = "09bf5ba0f67daac5b45a91ed1bea398e"
SRC_URI[sha256sum] = "0210f500c71af0ee7e8c42da76954298144d5f72f725ea381ae5db7b766b000e"

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