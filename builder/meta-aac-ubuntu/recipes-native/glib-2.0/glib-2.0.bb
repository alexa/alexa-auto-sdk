inherit native hostnativeglibpackage

do_install_append() {
	mkdir -p ${D}${includedir}
	cp -rpa ${includedir_native}/glib-2.0 ${D}${includedir}
	mkdir -p ${D}${libdir}/glib-2.0/include
	cp -rpa ${libdir_native}/${HOST_ARCH_NAME}/glib-2.0/include/glibconfig.h ${D}${libdir}/glib-2.0/include
}
