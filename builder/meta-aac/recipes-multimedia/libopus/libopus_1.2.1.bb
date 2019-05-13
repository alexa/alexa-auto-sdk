SUMMARY = "OPUS Audio Codec reference implementation"
HOMEPAGE = "https://opus-codec.org/"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://COPYING;md5=e304cdf74c2a1b0a33a5084c128a23a3"

SRC_URI = "https://archive.mozilla.org/pub/opus/opus-${PV}.tar.gz"
SRC_URI[md5sum] = "54bc867f13066407bc7b95be1fede090"
SRC_URI[sha256sum] = "cfafd339ccd9c5ef8d6ab15d7e1a412c054bf4cb4ecbbbcc78c12ef2def70732"

S = "${WORKDIR}/opus-${PV}"

inherit autotools pkgconfig

PACKAGECONFIG[ne10] = "--with-NE10=${STAGING_EXECPREFIXDIR},--without-NE10,ne10"
