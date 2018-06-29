# Unit and integration tests
testbindir = "${bindir}/tests"
FILES_${PN}-test = "${testbindir}/*"
PACKAGE_BEFORE_PN += "${PN}-test"
INSANE_SKIP_${PN}-test += "rpaths"