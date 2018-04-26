do_configure_android() {
	if [ "${ANDROID_ABI}" = "x86" ]; then
		# android-x86
		OPENSSL_MACHINE="i686"
		OPENSSL_TARGET="android-x86"
	else
		# android-armeabi
		OPENSSL_MACHINE="armv7"
		OPENSSL_TARGET="android-armeabi"
	fi
	sed -i -e "s/-mandroid//" Configurations/10-main.conf
	./Configure ${EXTRA_OECONF} \
	  ${OPENSSL_TARGET} \
	  -DARCH="${ANDROID_ABI}" \
	  -DCROSS_COMPILE="${TARGET_PREFIX}" \
	  -DMACHINE="${OPENSSL_MACHINE}" \
	  -DRELEASE="2.6.37" \
	  -DSYSTEM="android" \
	  no-asm \
	  no-comp \
	  no-dso \
	  no-dtls \
	  no-engine \
	  no-hw \
	  no-idea \
	  no-nextprotoneg \
	  no-psk \
	  no-shared \
	  no-srp \
	  no-ssl3 \
	  no-weak-ssl-ciphers \
	  --prefix=$prefix \
	  --openssldir=${libdir}/ssl-1.1 \
	  --libdir=`basename ${libdir}` \
	  -D_FORTIFY_SOURCE="2" -fstack-protector-strong
}

do_install_android() {
	oe_runmake DESTDIR="${D}" MANDIR="${mandir}" MANSUFFIX=ssl install
}