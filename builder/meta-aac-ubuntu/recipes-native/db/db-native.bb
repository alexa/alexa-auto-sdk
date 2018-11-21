inherit native hostnativepackage

do_install_append() {
	cp -P ${libdir_native}/${HOST_ARCH_NAME}/libdb-5.3.* ${D}${libdir}
}