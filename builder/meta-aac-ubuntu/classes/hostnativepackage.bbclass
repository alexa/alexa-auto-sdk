HOST_LIBRARY_NAME ?= "${BPN}"
HOST_INCLUDE_NAME ?= "${BPN}"
HOST_PKGCONFIG_NAME ?= "${BPN}"
HOST_ARCH_NAME ?= "x86_64-linux-gnu"

LICENSE ?= "Ubuntu"
do_populate_lic[noexec] = "1"

do_install() {
	mkdir -p ${D}${libdir}
	cp -P ${libdir_native}/${HOST_ARCH_NAME}/lib${HOST_LIBRARY_NAME}.* ${D}${libdir}
	if [ -e ${libdir_native}/${HOST_ARCH_NAME}/pkgconfig/${HOST_PKGCONFIG_NAME}.pc ]; then
		mkdir -p ${D}${libdir}/pkgconfig
		cp ${libdir_native}/${HOST_ARCH_NAME}/pkgconfig/${HOST_PKGCONFIG_NAME}.pc ${D}${libdir}/pkgconfig
	fi
	mkdir -p ${D}${includedir}
	cp ${includedir_native}/${HOST_INCLUDE_NAME}.h ${D}${includedir}
}