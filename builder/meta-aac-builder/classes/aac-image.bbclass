inherit image

# Override with the meta-aac layer version
PV = "${DISTRO_VERSION}"

IMAGE_NAME_SUFFIX = ""

# Workarounds
DEPENDS = ""
IMAGE_LINGUAS = ""
PACKAGE_INSTALL = "${IMAGE_INSTALL} ${FEATURE_INSTALL}"

IMAGE_PREPROCESS_COMMAND = "cleanup_rootfs ; generate_data_dir ;"

cleanup_rootfs() {
	rm -rf ${IMAGE_ROOTFS}${sysconfdir}
	rm -rf ${IMAGE_ROOTFS}${localstatedir}
	rm -rf ${IMAGE_ROOTFS}/run
}

generate_data_dir() {
	install -m 777 -d ${IMAGE_ROOTFS}${exec_prefix}/data
}