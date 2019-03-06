SUMMARY = "HTTP/2 C Library and tools"
HOMEPAGE = "https://nghttp2.org/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;beginline=6;md5=3fced11d6df719b47505837a51c16ae5"

SRC_URI = "https://github.com/nghttp2/nghttp2/releases/download/v${PV}/nghttp2-${PV}.tar.gz"
SRC_URI[md5sum] = "b68392c65f25241cc4aec9b26f51c0a7"
SRC_URI[sha256sum] = "66562e26c2a8112eb3d15e8521290352cbb5a8295271583840cb9a9f225c7195"

inherit cmake

FILES_${PN} += "${datadir}/${BPN}"

# Use OpenSSL from Homebrew install
EXTRA_OECMAKE_append_darwin = " -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl"