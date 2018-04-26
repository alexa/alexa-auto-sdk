#!/bin/bash
set -e

# Use GNU Tar
if [ $(uname) = "Darwin" ]; then
	[ -x /usr/local/bin/gfind ] && FIND="/usr/local/bin/gfind" || FIND="gfind"
	[ -x /usr/local/bin/gsed ] && SED="/usr/local/bin/gsed" || SED="gsed"
else
	FIND="find"
	SED="sed"
fi

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_HOME=${THIS_DIR}/../../..
BUILDER_HOME=${SDK_HOME}/builder

SYSROOTS=${THIS_DIR}/sysroots

mkdir -p ${SYSROOTS}

${FIND} ${BUILDER_HOME}/deploy \
-regextype sed \
-regex ".*aac-image-.*-android-[0-9]\+\.tar\.gz" \
-exec cp {} ${SYSROOTS} \;

for image in ${SYSROOTS}/*.tar.gz; do
	image_name=$(basename $image)
	echo "Updating sysroot for ${image_name}"
	filter="aac-image-[0-9a-z]\+-\(.\+\)-\(android-[0-9]\+\)\.tar\.gz"
	ANDROID_ABI=$(echo $image_name | ${SED} -n "s/${filter}/\1/p")
	ANDROID_PLATFORM=$(echo $image_name | ${SED} -n "s/${filter}/\2/p")
	SYSROOT="${SYSROOTS}/${ANDROID_ABI}/${ANDROID_PLATFORM}"
	mkdir -p ${SYSROOT}
	rm -rf ${SYSROOT}/*
	tar xf ${image} -C ${SYSROOT}
	echo "Sysroot updated: ABI=${ANDROID_ABI} PLATFORM=${ANDROID_PLATFORM}"
done