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

# Override with the aac-sdk version
PV = "${@aac.get_version(d)}"

HOMEPAGE ?= "http://github.com/alexa/aac-sdk"
LICENSE ?= "CLOSED"

inherit cmake unittests

FILES_${PN}-dev += "${datadir}/cmake"
EXTRA_OECMAKE += "-DAAC_HOME=${STAGING_DIR_HOST}${AAC_PREFIX} \
                  -DCMAKE_BUILD_TYPE=${AAC_BUILD_TYPE} \
                  -DAAC_EMIT_SENSITIVE_LOGS=${AAC_SENSITIVE_LOGS} \
                  -DAAC_EMIT_LATENCY_LOGS=${AAC_LATENCY_LOGS} \
                  -DAAC_ENABLE_TESTS=${AAC_ENABLE_TESTS} \
                  -DAAC_VERSION=${PV}"

# Install AAC artifacts explicitly into AAC_PREFIX dir
EXTRA_OECMAKE_append = " -DCMAKE_INSTALL_PREFIX:PATH=${AAC_PREFIX}"
FILES_${PN} = "${AAC_PREFIX}/*"
SYSROOT_DIRS_append = " ${AAC_PREFIX}"

# Add search path for AVS Device SDK
OECMAKE_EXTRA_ROOT_PATH = "${STAGING_DIR_HOST}${AAC_PREFIX}"

do_install_append() {
	mkdir -p ${D}${testbindir}/${PN}
	find ./ -name "*Test" -exec cp {} ${D}${testbindir}/${PN} \;
}
