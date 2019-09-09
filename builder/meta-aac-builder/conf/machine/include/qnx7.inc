QNX_BASE ?= "${HOME}/qnx700"
QNX_ARCH ?= "x86_64"

export QNX_HOST = "${QNX_BASE}/host/linux/${BUILD_ARCH}"
export QNX_TARGET = "${QNX_BASE}/target/qnx7"
export QNX_CONFIGURATION = "${HOME}/.qnx"

TCLIBC = "qnx"
TCMODE = "external"

TARGET_OS = "nto-qnx7.0.0${ABIEXTENSION}"
TARGET_VENDOR = "${@bb.utils.contains_any('TARGET_ARCH', 'arm aarch64', '-unknown', '-pc', d)}"

# CMake
OECMAKE_EXTRA_ROOT_PATH += "${QNX_TARGET} ${QNX_TARGET}/${QNX_ARCH}"
AAC_CMAKE_SYSTEM_NAME = "QNX"

# No toolchain options
TOOLCHAIN_OPTIONS = ""

# Override tune args
TUNE_CCARGS = "-Vgcc_nto${QNX_ARCH}"

# Use QCC rather than GCC
export CC = "qcc ${TUNE_CCARGS}"
export CXX = "q++ ${TUNE_CCARGS}"
export CPP = "qcc ${TUNE_CCARGS} -E"

# Add toolchain path
PATH_prepend = "${QNX_HOST}/usr/bin:"
EXTERNAL_TOOLCHAIN = "${QNX_HOST}"

TARGET_CFLAGS = ""
# Use LLVM variant
TARGET_CXXFLAGS = " -Y_cxx"
TARGET_LDFLAGS = ""

# pkg-config
export AAC_PKG_CONFIG_SYSROOT_DIR = "${QNX_TARGET}"
export AAC_PKG_CONFIG_LIBDIR = "${AAC_HOSTTOOLS}/qnx7-pkgconfig/${QNX_ARCH}"

# Workaround
MODIFYTOS = "0"

##################################################################
# OE extra functions
##################################################################

def qnx7_siteinfo_setfunc(archinfo, osinfo, targetinfo, d):
    osinfo['nto-qnx7.0.0'] = "common-qnx"
    targetinfo['x86_64-nto-qnx7.0.0'] = "bit-64"
    return archinfo, osinfo, targetinfo

SITEINFO_EXTRA_DATAFUNCS = "qnx7_siteinfo_setfunc"

def qnx7_get_machine_dict(machdata, d):
    machdata["nto-qnx7.0.0"] = {
        "x86_64": (62,  0, 0, True, 64),
        "aarch64":(183, 0, 0, True, 64),
    }
    return machdata

PACKAGEQA_EXTRA_MACHDEFFUNCS = "qnx7_get_machine_dict"
