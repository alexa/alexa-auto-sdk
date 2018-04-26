python () {
    if d.getVar('SRC_URI'):
        return
    d.setVar('SRC_URI', "file://.;subdir=src")
    d.setVar('S', "${WORKDIR}/src")
    module_path = os.path.dirname(d.getVar('FILE'))
    d.prependVar('FILESEXTRAPATHS', module_path)
    d.setVar('MODULE_PATH', module_path)
}

inherit cmake

FILES_${PN}-dev += "${datadir}/cmake"
EXTRA_OECMAKE += "-DAAC_HOME=${STAGING_DIR_HOST}${prefix} -DCMAKE_BUILD_TYPE=${OECMAKE_BUILD_TYPE}"