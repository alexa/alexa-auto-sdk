python () {
    if d.getVar('SRC_URI'):
        return
    d.setVar('SRC_URI', "file://.;subdir=src")
    d.setVar('S', "${WORKDIR}/src")
    module_path = os.path.dirname(d.getVar('FILE'))
    d.prependVar('FILESEXTRAPATHS', module_path)
    d.setVar('MODULE_PATH', module_path)
}

# Override with the meta-aac layer version
PV = "${DISTRO_VERSION}"

HOMEPAGE ?= "http://github.com/alexa/aac-sdk"
LICENSE ?= "CLOSED"

inherit cmake unittests

FILES_${PN}-dev += "${datadir}/cmake"
EXTRA_OECMAKE += "-DAAC_HOME=${STAGING_DIR_HOST}${prefix} \
                  -DCMAKE_BUILD_TYPE=${OECMAKE_BUILD_TYPE} \
                  -DAAC_EMIT_SENSITIVE_LOGS=${AAC_SENSITIVE_LOGS} \
                  -DAAC_LATENCY_LOGS=${AAC_LATENCY_LOGS} \
                  -DAAC_ENABLE_TESTS=ON \
                  -DAAC_VERSION=${PV}"

do_install_append() {
	mkdir -p ${D}${testbindir}/${PN}
	find ./ -name "*Test" -exec cp {} ${D}${testbindir}/${PN} \;
}
