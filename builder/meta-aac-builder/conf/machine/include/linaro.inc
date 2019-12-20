LINARO_TOOLCHAIN_PREFIX ?= "${HOME}/gcc-linaro-7.4.1-2019.02-"
LINARO_SYSROOTS ?= "${HOME}/sysroots"

TARGET_VENDOR = ""
TARGET_SYSROOT = "${LINARO_SYSROOTS}/${TARGET_SYS}"

EXTERNAL_TOOLCHAIN = "${LINARO_TOOLCHAIN_PREFIX}${BUILD_ARCH}_${TARGET_SYS}"

# CMake
OECMAKE_EXTRA_ROOT_PATH += "${TARGET_SYSROOT}"

# Add sysroot option
TOOLCHAIN_OPTIONS = " --sysroot=${TARGET_SYSROOT}"

# Add toolchain path
PATH_prepend = "${EXTERNAL_TOOLCHAIN}/bin:"

# Workaround
MODIFYTOS = "0"

# Assuming GStreamer is provided from host
AAC_PKG_CONFIG_PROVIDED_append = " gstreamer-1.0 gstreamer-app-1.0"
AAC_ASSUME_PROVIDED_append = " gstreamer1.0"

# pkg-config
export AAC_PKG_CONFIG_SYSROOT_DIR = "${TARGET_SYSROOT}"
export AAC_PKG_CONFIG_LIBDIR = "${TARGET_SYSROOT}/usr/lib/pkgconfig"
