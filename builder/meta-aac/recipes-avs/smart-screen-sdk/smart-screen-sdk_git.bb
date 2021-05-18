SUMMARY = "Smart Screen SDK (private beta)"
HOMEPAGE = "https://github.com/alexa/alexa-smart-screen-sdk"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE.TXT;md5=8af6ce427017dadf03f47608d86f3716"

DEPENDS = "avs-device-sdk"

SRC_URI = "git://github.com/alexa/alexa-smart-screen-sdk-for-linux.git"
SRCREV = "0ebddc18587e32cc3c3bbadeb169cc907a565845"
S = "${WORKDIR}/git"

SRC_URI += "file://0001-Smart-Screen-SDK-for-Alexa-Auto-SDK.patch \
            file://0002-Disable-SmartScreenCapabilityAgents-test.patch \
            file://0003-Disable-APLClient-dependency.patch"

EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=${AAC_BUILD_TYPE}"

OECMAKE_SOURCEPATH = "${S}/modules/Alexa"

inherit pkgconfig cmake devlibsonly

do_install_append() {
	chrpath -d ${D}${libdir}/libVisualCharacteristics.so
}
