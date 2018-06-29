inherit image

# Override with the meta-aac layer version
PV = "${DISTRO_VERSION}"

IMAGE_NAME_SUFFIX = ""

# Workarounds
DEPENDS = ""
IMAGE_LINGUAS = ""
PACKAGE_INSTALL = "${IMAGE_INSTALL} ${FEATURE_INSTALL}"

IMAGE_PREPROCESS_COMMAND = "cleanup_rootfs ; prepare_release ;"

cleanup_rootfs() {
	rm -rf ${IMAGE_ROOTFS}${sysconfdir}
	rm -rf ${IMAGE_ROOTFS}${localstatedir}
	rm -rf ${IMAGE_ROOTFS}/run
}

prepare_release() {
	if [ -d ${IMAGE_ROOTFS}${releasedir} ]; then
		cp -R ${IMAGE_ROOTFS}${releasedir}/* ${IMAGE_ROOTFS}${exec_prefix}
		rm -rf ${IMAGE_ROOTFS}${releasedir}
	fi
}