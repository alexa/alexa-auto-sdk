SUMMARY = "Snowboy Hotword Detection by KITT.AI"
HOMEPAGE = "https://snowboy.kitt.ai/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8d385adc93db3f7e5d092b82fb2c9353"

SRC_URI = "https://github.com/Kitt-AI/snowboy/archive/v${PV}.tar.gz"
SRC_URI[md5sum] = "e16e117914f76f4a35facb16b0fcd61b"
SRC_URI[sha256sum] = "93658cc5d4d83dd7a6e954d0dc1e18d1c70b1dd8623181850c4804d3aca19e96"

COMPATIBLE_MACHINE = "(machine-native|android)"

INSANE_SKIP_${PN} += "already-stripped"

def get_snowboy_arch(d):
    machine = d.getVar('MACHINE')
    if machine == "androidarm":
        return "android/armv7a"
    elif machine == "androidarm64":
        return "android/armv8-aarch64"
    else:
        return "ubuntu64"

SNOWBOY_ARCH ?= "${@get_snowboy_arch(d)}"

do_install() {
	mkdir -p ${D}${libdir}
	cp ${S}/lib/${SNOWBOY_ARCH}/libsnowboy-detect.a ${D}${libdir}
	mkdir -p ${D}${includedir}
	cp ${S}/include/snowboy-detect.h ${D}${includedir}
}