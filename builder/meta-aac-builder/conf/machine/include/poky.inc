POKY_SDK ??= "/opt/poky/2.4.4"
POKY_DISTRO ?= "poky"

TARGET_VENDOR = "-poky"
TARGET_SYSROOT = "${POKY_SDK}/sysroots/${MULTIMACH_TARGET_SYS}"
OECORE_NATIVE_SYSROOT = "${POKY_SDK}/sysroots/x86_64-${POKY_DISTRO}sdk-linux"

# CMake
OECMAKE_EXTRA_ROOT_PATH += "${TARGET_SYSROOT}"

# Use external OE toolchain
TOOLCHAIN_OPTIONS = " --sysroot=${TARGET_SYSROOT}"

# Add toolchain path
PATH_prepend = "${OECORE_NATIVE_SYSROOT}/usr/bin/${TARGET_SYS}:"
EXTERNAL_TOOLCHAIN = "${OECORE_NATIVE_SYSROOT}"

# Workaround
MODIFYTOS = "0"

# pkg-config
export AAC_PKG_CONFIG_SYSROOT_DIR = "${TARGET_SYSROOT}"
export AAC_PKG_CONFIG_LIBDIR = "${TARGET_SYSROOT}/usr/lib/pkgconfig"

# Adding Poky universal override
MACHINEOVERRIDES = "${MACHINE}:poky"
