TARGET_VENDOR = "-agl"

AGL_SDK ?= "/opt/poky-agl/5.1.0"

TARGET_SYSROOT = "${AGL_SDK}/sysroots/${TARGET_SYS}"
OECORE_NATIVE_SYSROOT = "${AGL_SDK}/sysroots/x86_64-aglsdk-linux"

# CMake
OECMAKE_EXTRA_ROOT_PATH += "${TARGET_SYSROOT}"
# Newer version of LD linker has some strange behavior: -rpath will be
# ignored to search indirect dependencies. Only -rpath-link seems to
# be working.
OECMAKE_C_LINK_FLAGS += "-Wl,-rpath-link,${STAGING_DIR_HOST}${libdir}"
OECMAKE_CXX_LINK_FLAGS += "-Wl,-rpath-link,${STAGING_DIR_HOST}${libdir}"

# Use external OE toolchain
TOOLCHAIN_OPTIONS = " --sysroot=${TARGET_SYSROOT}"

# Add toolchain path
PATH_prepend = "${OECORE_NATIVE_SYSROOT}/usr/bin/${TARGET_SYS}:"
EXTERNAL_TOOLCHAIN = "${OECORE_NATIVE_SYSROOT}"

# Adding AGL universal override
MACHINEOVERRIDES = "${MACHINE}:agl"

# Workaround
MODIFYTOS = "0"

##################################################################
# Qt5
##################################################################

QMAKE ?= "${OECORE_NATIVE_SYSROOT}/usr/bin/qt5/qmake"

export OE_QMAKE_CFLAGS = "${CFLAGS}"
export OE_QMAKE_CXXFLAGS = "${CXXFLAGS}"
export OE_QMAKE_LDFLAGS = "${LDFLAGS}"
export OE_QMAKE_CC = "${CC}"
export OE_QMAKE_CXX = "${CXX}"
export OE_QMAKE_LINK = "${CXX}"
export OE_QMAKE_AR = "${AR}"
export QT_CONF_PATH = "${OECORE_NATIVE_SYSROOT}/usr/bin/qt5/qt.conf"
export OE_QMAKE_LIBDIR_QT = "`${QMAKE} -query QT_INSTALL_LIBS`"
export OE_QMAKE_INCDIR_QT = "`${QMAKE} -query QT_INSTALL_HEADERS`"
export OE_QMAKE_MOC = "${OECORE_NATIVE_SYSROOT}/usr/bin/qt5/moc"
export OE_QMAKE_UIC = "${OECORE_NATIVE_SYSROOT}/usr/bin/qt5/uic"
export OE_QMAKE_RCC = "${OECORE_NATIVE_SYSROOT}/usr/bin/qt5/rcc"
export OE_QMAKE_QDBUSCPP2XML = "${OECORE_NATIVE_SYSROOT}/usr/bin/qt5/qdbuscpp2xml"
export OE_QMAKE_QDBUSXML2CPP = "${OECORE_NATIVE_SYSROOT}/usr/bin/qt5/qdbusxml2cpp"
export OE_QMAKE_QT_CONFIG = "`${QMAKE} -query QT_INSTALL_LIBS`/qt5/mkspecs/qconfig.pri"
export OE_QMAKE_PATH_HOST_BINS = "${OECORE_NATIVE_SYSROOT}/usr/bin/qt5"
export QMAKESPEC = "`${QMAKE} -query QT_INSTALL_LIBS`/qt5/mkspecs/linux-oe-g++"
