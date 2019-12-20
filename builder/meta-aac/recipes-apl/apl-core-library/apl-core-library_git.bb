SUMMARY = "APL Core Library (private beta)"
HOMEPAGE = "https://github.com/alexa/apl-core-library"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=d92e60ee98664c54f68aa515a6169708"

SRC_URI = "git://github.com/alexa/apl-core-library.git;protocol=http"
SRCREV = "f0a99732c530fde9d9fac97c2d008c012594fc51"
S = "${WORKDIR}/git"

inherit autotools pkgconfig cmake devlibsonly

EXTRA_OECMAKE += "-DBUILD_TESTS=OFF \
                  -DCOVERAGE=OFF \
                  -DVERBOSE=1"

do_compile_prepend_android() {
	export CROSS_SYSROOT="${TARGET_SYSROOT}"
}

do_install() {
	# Note: we cannot export this package because it is not installed
	# # make: *** No rule to make target `install'.  Stop.
	mkdir -p ${D}${libdir}
	cp ${B}/aplcore/libapl.a ${D}${libdir}
	cp ${B}/lib/libyogacore.a ${D}${libdir}
	mkdir -p ${D}${includedir}
	cp -R ${S}/aplcore/include/ ${D}${includedir}
	cp -R ${B}/yoga-prefix/src/yoga/yoga ${D}${includedir}
}
