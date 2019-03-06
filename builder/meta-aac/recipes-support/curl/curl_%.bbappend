
# ngHTTP2 is required by AVS Device SDK
#PACKAGECONFIG_class-target_append = " nghttp2"
PACKAGECONFIG ?= "${@bb.utils.filter('DISTRO_FEATURES', 'ipv6', d)} gnutls proxy threaded-resolver zlib nghttp2"

do_configure_append() {
	# Configure script will try to find SSL by default.
	# Should indicate as error if SSL couldn't be found.
	ssl_enabled=$(grep "SSL_ENABLED" ${B}/config.status)
	if [ ! "$ssl_enabled" = 'S["SSL_ENABLED"]="1"' ]; then
		echo "SSL is not enabled!"
		exit 1
	fi
}