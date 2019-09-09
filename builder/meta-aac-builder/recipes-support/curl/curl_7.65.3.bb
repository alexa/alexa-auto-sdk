SUMMARY = "Command line tool and library for client-side URL transfers"
HOMEPAGE = "http://curl.haxx.se/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;beginline=8;md5=3a34942f4ae3fbf1a303160714e664ac"

SRC_URI = "http://curl.haxx.se/download/curl-${PV}.tar.bz2"
SRC_URI[md5sum] = "bdd220d018c23e7f9ee82894e62a70bc"
SRC_URI[sha256sum] = "0a855e83be482d7bc9ea00e05bdb1551a44966076762f9650959179c89fce509"

INHIBIT_DEFAULT_DEPS = "1"

inherit autotools pkgconfig

# SSL & ngHTTP2 is required by AVS Device SDK
PACKAGECONFIG = "ssl nghttp2"

# Use OpenSSL explicitly for Android
SSL_OPTION_android = "--with-ssl=${STAGING_DIR_HOST}${prefix}"
SSL_PACKAGE_android = "openssl"
# Use system default SSL library for other platforms
SSL_OPTION = ""
SSL_PACKAGE = ""

PACKAGECONFIG[ssl] = "${SSL_OPTION},--without-ssl,${SSL_PACKAGE}"
PACKAGECONFIG[mbedtls] = "--with-mbedtls=${STAGING_DIR_HOST}${prefix},--without-mbedtls,mbedtls"
PACKAGECONFIG[nghttp2] = "--with-nghttp2,--without-nghttp2,nghttp2"
