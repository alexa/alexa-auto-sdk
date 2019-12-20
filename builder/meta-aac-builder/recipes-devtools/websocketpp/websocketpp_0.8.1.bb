SUMMARY = "C++ websocket client/server library"
HOMEPAGE = "https://github.com/zaphoyd/websocketpp"
LICENSE = ""
LIC_FILES_CHKSUM = "file://COPYING;md5=4d168d763c111f4ffc62249870e4e0ea"

SRC_URI = "https://github.com/zaphoyd/websocketpp/archive/${PV}.tar.gz"
SRC_URI[md5sum] = "75e96b15ba1cb3b37175d58352c3a7fb"
SRC_URI[sha256sum] = "178899de48c02853b55b1ea8681599641cedcdfce59e56beaff3dd0874bf0286"

S = "${WORKDIR}/websocketpp-${PV}"

do_install() {
	mkdir -p ${D}${includedir}
	cp -R ${S}/websocketpp ${D}${includedir}
}
