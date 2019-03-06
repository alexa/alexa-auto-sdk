SUMMARY = "Command line tool and library for client-side URL transfers"
HOMEPAGE = "http://curl.haxx.se/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;beginline=8;md5=3a34942f4ae3fbf1a303160714e664ac"

SRC_URI = "http://curl.haxx.se/download/curl-${PV}.tar.bz2"
SRC_URI[md5sum] = "fa049f9f90c1ae473a2a7bcfa14de976"
SRC_URI[sha256sum] = "1cb081f97807c01e3ed747b6e1c9fee7a01cb10048f1cd0b5f56cfe0209de731"

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
