SUMMARY = "Command line tool and library for client-side URL transfers"
HOMEPAGE = "http://curl.haxx.se/"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;beginline=8;md5=3a34942f4ae3fbf1a303160714e664ac"

DEPENDS = "nghttp2"
DEPENDS_append_android = " openssl"

SRC_URI = "http://curl.haxx.se/download/curl-${PV}.tar.bz2"
SRC_URI[md5sum] = "e0caf257103e0c77cee5be7e9ac66ca4"
SRC_URI[sha256sum] = "de60a4725a3d461c70aa571d7d69c788f1816d9d1a8a2ef05f864ce8f01279df"

INHIBIT_DEFAULT_DEPS = "1"

inherit autotools pkgconfig

EXTRA_OECONF += "--with-nghttp2=${STAGING_DIR_HOST}${prefix}"

# Use OpenSSL from Homebrew install
EXTRA_OECONF_append_darwin = " --with-ssl=/usr/local/opt/openssl"
# Use OpenSSL from our sysroot
EXTRA_OECONF_append_android = " --with-ssl=${STAGING_DIR_HOST}${prefix}"

# Use CA certs from AGL installation
EXTRA_OECONF_append_m3ulcb = " --with-ca-bundle=/usr/lib/ssl/certs/ca-certificates.crt"

do_configure_append() {
	# Configure script will try to find SSL by default.
	# Should be indicated as error if SSL couldn't be found.
	ssl_enabled=$(grep "SSL_ENABLED" ${B}/config.status)
	if [ ! "$ssl_enabled" = 'S["SSL_ENABLED"]="1"' ]; then
		echo "SSL is not enabled!"
		exit 1
	fi
}
