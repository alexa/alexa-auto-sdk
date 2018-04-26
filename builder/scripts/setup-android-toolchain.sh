#!/bin/bash
set -e

#
# ./setup-android-toolchains.sh <abi> <api-level>
#

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILDER_HOME=${THIS_DIR}/..

# External values
: ${ANDROID_TOOLCHAIN:="${BUILDER_HOME}/android-toolchain"}
: ${NDK_PACKAGE:="android-ndk-r16b"}

# Extra options
ANDROID_ABI=${1}
API_LEVEL=${2}

if [ -z ${ANDROID_ABI} ] || [ -z ${API_LEVEL} ]; then
	echo "Please specify ABI/API"
	exit 1
fi

# Android toolchain path
ANDROID_NDK="${ANDROID_TOOLCHAIN}/ndk"
ANDROID_SDK="${ANDROID_TOOLCHAIN}/sdk"

# Standard Android path
ANDROID_NDK_HOME="${ANDROID_NDK}/ndk-bundle/${NDK_PACKAGE}"

# OS detection
if [ $(uname) = "Darwin" ]; then
	HOST="darwin"
else
	HOST="linux"
fi

install_ndk() {
	echo "Installing NDK (${NDK_PACKAGE})..."
	NDK_PACKAGE_FILE="${ANDROID_TOOLCHAIN}/${NDK_PACKAGE}-${HOST}-x86_64.zip"
	if [ ! -e ${NDK_PACKAGE_FILE} ]; then
		# Download Android NDK
		wget https://dl.google.com/android/repository/${NDK_PACKAGE}-${HOST}-x86_64.zip -P ${ANDROID_TOOLCHAIN}
	fi
	mkdir -p ${ANDROID_NDK}/ndk-bundle
	pushd ${ANDROID_NDK}/ndk-bundle
	unzip -q ${NDK_PACKAGE_FILE}
	popd
}

install_sdk_tools() {
	echo "Installing SDK Tools..."
	SDK_TOOLS_FILE="${ANDROID_TOOLCHAIN}/sdk-tools-${HOST}-3859397.zip"
	if [ ! -e ${SDK_TOOLS_FILE} ]; then
		wget https://dl.google.com/android/repository/sdk-tools-${HOST}-3859397.zip -P ${ANDROID_TOOLCHAIN}
	fi
	mkdir -p ${ANDROID_SDK}
	pushd ${ANDROID_SDK}
	unzip -q ${SDK_TOOLS_FILE}
	popd
}

generate_toolchain() {
	case ${ANDROID_ABI} in
		x86)
			ARCH="x86"
			;;
		armeabi-v7a)
			ARCH="arm"
			;;
		*)
			echo "Unknown ABI: ${ANDROID_ABI}"
			exit 1
	esac

	TOOLCHAIN="${ANDROID_NDK}/toolchains/${NDK_PACKAGE}/toolchain-${ANDROID_ABI}/android-${API_LEVEL}"

	if [ ! -d ${TOOLCHAIN} ]; then
		echo "Generating Standalone Toolchain..."

		${ANDROID_NDK}/ndk-bundle/${NDK_PACKAGE}/build/tools/make_standalone_toolchain.py \
		--arch="${ARCH}" \
		--api="${API_LEVEL}" \
		--stl="libc++" \
		--force \
		--install-dir="${TOOLCHAIN}"
	fi
}

echo "Checking Android toolchain installation (${ANDROID_ABI}/${API_LEVEL})..."

if [ ! -d ${ANDROID_NDK_HOME} ]; then
	install_ndk
fi
if [ ! -d ${ANDROID_SDK} ]; then
	install_sdk_tools
fi

generate_toolchain