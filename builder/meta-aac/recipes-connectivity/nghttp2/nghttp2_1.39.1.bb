SUMMARY = "HTTP/2 C Library and tools"
HOMEPAGE = "https://nghttp2.org/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;beginline=6;md5=3fced11d6df719b47505837a51c16ae5"

SRC_URI = "https://github.com/nghttp2/nghttp2/releases/download/v${PV}/nghttp2-${PV}.tar.gz"
SRC_URI[md5sum] = "fcfe5362ce9956bb6070c1ee511d6928"
SRC_URI[sha256sum] = "25b623cd04dc6a863ca3b34ed6247844effe1aa5458229590b3f56a6d53cd692"

inherit cmake

FILES_${PN} += "${datadir}/${BPN}"

# Use OpenSSL from Homebrew install
EXTRA_OECMAKE_append_darwin = " -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl"