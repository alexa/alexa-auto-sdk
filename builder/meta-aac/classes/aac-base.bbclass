python () {
    if not bb.data.inherits_class('native', d) and not bb.data.inherits_class('cross', d):
        d.delVar('PKG_CONFIG_PATH')
        d.delVar('PKG_CONFIG_SYSTEM_LIBRARY_PATH')
        d.delVar('PKG_CONFIG_SYSTEM_INCLUDE_PATH')
}

modify_find_root_path_mode() {
    sed -e 's/\(CMAKE_FIND_ROOT_PATH_MODE_[A-Z]\+\) \([A-Z]\+\)/\1 BOTH/g' \
        -i ${WORKDIR}/toolchain.cmake
}

modify_system_name() {
    sed -e 's/\(CMAKE_SYSTEM_NAME\) \([-A-Za-z0-9\.]\+\)/\1 ${AAC_CMAKE_SYSTEM_NAME}/g' \
        -i ${WORKDIR}/toolchain.cmake
}

python do_tweak_toolchain_file() {
    if bb.data.inherits_class('native', d) or bb.data.inherits_class('cross', d):
        return
    if not bb.data.inherits_class('cmake', d):
        return
    overrides = d.getVar('OVERRIDES').split(':')
    if "machine-native" in overrides:
        bb.build.exec_func("modify_find_root_path_mode", d)
    elif "qnx" in overrides:
        bb.build.exec_func("modify_system_name", d)
}
addtask tweak_toolchain_file after do_generate_toolchain_file before do_configure