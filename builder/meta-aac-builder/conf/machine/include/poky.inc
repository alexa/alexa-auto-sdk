POKY_SDK ??= "/opt/poky/2.6.1"
POKY_DISTRO ?= "poky"

TARGET_VENDOR = "-poky"
TARGET_SYSROOT = "${POKY_SDK}/sysroots/${MULTIMACH_TARGET_SYS}"
OECORE_NATIVE_SYSROOT = "${POKY_SDK}/sysroots/x86_64-${POKY_DISTRO}sdk-linux"

# Newer version of LD linker has some strange behavior: -rpath will be
# ignored to search indirect dependencies. Only -rpath-link seems to
# be working.
OECMAKE_C_LINK_FLAGS += "-Wl,-rpath-link,${STAGING_DIR_HOST}${libdir}"
OECMAKE_CXX_LINK_FLAGS += "-Wl,-rpath-link,${STAGING_DIR_HOST}${libdir}"

# CMake
OECMAKE_EXTRA_ROOT_PATH += "${TARGET_SYSROOT}"

# Use external OE toolchain
TOOLCHAIN_OPTIONS = " --sysroot=${TARGET_SYSROOT}"

# Add toolchain path
PATH_prepend = "${OECORE_NATIVE_SYSROOT}/usr/bin/${TARGET_SYS}:"
EXTERNAL_TOOLCHAIN = "${OECORE_NATIVE_SYSROOT}"

# Workaround
MODIFYTOS = "0"

# Assuming GStreamer is provided from host
AAC_PKG_CONFIG_PROVIDED_append = " gstreamer-1.0 gstreamer-app-1.0"
AAC_ASSUME_PROVIDED_append = " ^gstreamer1.0$"

# pkg-config
export AAC_PKG_CONFIG_SYSROOT_DIR = "${TARGET_SYSROOT}"
export AAC_PKG_CONFIG_LIBDIR = "${TARGET_SYSROOT}/usr/lib/pkgconfig"
