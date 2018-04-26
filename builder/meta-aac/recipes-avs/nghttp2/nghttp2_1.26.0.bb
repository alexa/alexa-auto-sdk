SUMMARY = "HTTP/2 C Library and tools"
HOMEPAGE = "https://nghttp2.org/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;beginline=6;md5=3fced11d6df719b47505837a51c16ae5"

SRC_URI = "https://github.com/nghttp2/nghttp2/releases/download/v${PV}/nghttp2-${PV}.tar.gz"
SRC_URI[md5sum] = "83fa813b22bacbc6ea80dfb24847569f"
SRC_URI[sha256sum] = "daf7c0ca363efa25b2cbb1e4bd925ac4287b664c3d1465f6a390359daa3f0cf1"

inherit cmake

FILES_${PN}_remove = "${datadir}/${BPN}"

# Use OpenSSL from Homebrew install
EXTRA_OECMAKE_append_darwin = " -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl"