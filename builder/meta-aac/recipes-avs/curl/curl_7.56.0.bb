SUMMARY = "Command line tool and library for client-side URL transfers"
HOMEPAGE = "http://curl.haxx.se/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;beginline=8;md5=3a34942f4ae3fbf1a303160714e664ac"

SRC_URI = "http://curl.haxx.se/download/curl-${PV}.tar.bz2"
SRC_URI[md5sum] = "e0caf257103e0c77cee5be7e9ac66ca4"
SRC_URI[sha256sum] = "de60a4725a3d461c70aa571d7d69c788f1816d9d1a8a2ef05f864ce8f01279df"

INHIBIT_DEFAULT_DEPS = "1"

inherit autotools pkgconfig

# SSL & ngHTTP2 is required by AVS Device SDK
PACKAGECONFIG ??= "ssl nghttp2"

# Use OpenSSL explicitly for Android
SSL_OPTION_android = "--with-ssl=${STAGING_DIR_HOST}${prefix}"
SSL_PACKAGE_android = "openssl"
# Use system default SSL library for other platforms
SSL_OPTION = ""
SSL_PACKAGE = ""

PACKAGECONFIG[ssl] = "${SSL_OPTION},--without-ssl,${SSL_PACKAGE}"
PACKAGECONFIG[mbedtls] = "--with-mbedtls=${STAGING_DIR_HOST}${prefix},--without-mbedtls,mbedtls"
PACKAGECONFIG[nghttp2] = "--with-nghttp2,--without-nghttp2,nghttp2"

do_configure_append() {
	# Configure script will try to find SSL by default.
	# Should indicate as error if SSL couldn't be found.
	ssl_enabled=$(grep "SSL_ENABLED" ${B}/config.status)
	if [ ! "$ssl_enabled" = 'S["SSL_ENABLED"]="1"' ]; then
		echo "SSL is not enabled!"
		exit 1
	fi
}
