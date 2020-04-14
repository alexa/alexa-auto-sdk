python () {
    if d.getVar('SRC_URI'):
        return
    d.setVar('SRC_URI', "file://.;subdir=src")
    d.appendVar('SRC_URI', " " + d.getVar('AAC_PATCHES'))
    d.setVar('S', "${WORKDIR}/src")
    module_path = os.path.dirname(d.getVar('FILE'))
    d.prependVar('FILESEXTRAPATHS', module_path + ":")
    d.setVar('MODULE_PATH', module_path)
}

AAC_PATCHES ??= ""

# Override with the sdk version
PV = "${@aac.get_version(d)}"

HOMEPAGE ?= "http://github.com/alexa/alexa-auto-sdk"
LICENSE ?= "CLOSED"

inherit cmake unittests

FILES_${PN}-dev += "${datadir}/cmake"
EXTRA_OECMAKE += "-DAAC_HOME=${STAGING_DIR_HOST}${AAC_PREFIX} \
                  -DCMAKE_BUILD_TYPE=${AAC_BUILD_TYPE} \
                  -DAAC_EMIT_SENSITIVE_LOGS=${AAC_SENSITIVE_LOGS} \
                  -DAAC_EMIT_LATENCY_LOGS=${AAC_LATENCY_LOGS} \
                  -DAAC_VERSION=${PV} \
                  -DAAC_ENABLE_COVERAGE=${AAC_ENABLE_COVERAGE} \
                  -DAAC_ENABLE_ADDRESS_SANITIZER=${AAC_ENABLE_ADDRESS_SANITIZER}"

# Install AAC artifacts explicitly into AAC_PREFIX dir
EXTRA_OECMAKE_append = " -DCMAKE_INSTALL_PREFIX:PATH=${AAC_PREFIX}"
FILES_${PN} = "${AAC_PREFIX}/*"
SYSROOT_DIRS_append = " ${AAC_PREFIX}"

# Add search path for AVS Device SDK
OECMAKE_EXTRA_ROOT_PATH = "${STAGING_DIR_HOST}${AAC_PREFIX}"

# Unit tests configuration
PACKAGECONFIG[test] = "-DAAC_ENABLE_TESTS=ON,,googletest"

# Enable & install test packages if AAC_ENABLE_TESTS is true
PACKAGECONFIG += "${@['', 'test'][(d.getVar('AAC_ENABLE_TESTS') == '1')]}"
RDEPENDS_${PN} += "${@['', '${PN}-test'][(d.getVar('AAC_ENABLE_TESTS') == '1')]}"

do_install_append() {
	mkdir -p ${D}${testbindir}/${PN}
	find ./ -regex ".*Tests?" -exec cp {} ${D}${testbindir}/${PN} \;
}
