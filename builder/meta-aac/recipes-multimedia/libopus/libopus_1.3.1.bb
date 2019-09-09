SUMMARY = "OPUS Audio Codec reference implementation"
HOMEPAGE = "https://opus-codec.org/"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://COPYING;md5=e304cdf74c2a1b0a33a5084c128a23a3"

SRC_URI = "https://archive.mozilla.org/pub/opus/opus-${PV}.tar.gz"
SRC_URI[md5sum] = "d7c07db796d21c9cf1861e0c2b0c0617"
SRC_URI[sha256sum] = "65b58e1e25b2a114157014736a3d9dfeaad8d41be1c8179866f144a2fb44ff9d"

S = "${WORKDIR}/opus-${PV}"

inherit autotools pkgconfig

PACKAGECONFIG[ne10] = "--with-NE10=${STAGING_EXECPREFIXDIR},--without-NE10,ne10"
