#!/bin/bash
set -e

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_HOME=${THIS_DIR}/../../..
BUILDER_HOME=${SDK_HOME}/builder

DEPLOY_DIR=${THIS_DIR}/native-dependencies

# Extra options
ANDROID_ABI=${1}
ANDROID_PLATFORM=${2}

if [ -z ${ANDROID_ABI} ] || [ -z ${ANDROID_PLATFORM} ]; then
	echo "Please specify ABI/API"
	exit 1
fi

mkdir -p ${DEPLOY_DIR}

IMAGE_NAME="aac-image-minimal-${ANDROID_ABI}-${ANDROID_PLATFORM}.tar.gz"

find ${BUILDER_HOME}/deploy \
-name ${IMAGE_NAME} \
-exec cp {} ${DEPLOY_DIR} \;

if [ ! -e ${DEPLOY_DIR}/${IMAGE_NAME} ]; then
	echo "AAC Build for ${ANDROID_ABI}/${ANDROID_PLATFORM} is not found!"
	exit 1
fi

SYSROOT="${DEPLOY_DIR}/sysroots/${ANDROID_ABI}"
mkdir -p ${SYSROOT} && tar xf ${DEPLOY_DIR}/${IMAGE_NAME} -C ${SYSROOT}

JNI_DEPLOY="${DEPLOY_DIR}/jni/${ANDROID_ABI}"
mkdir -p ${JNI_DEPLOY}
find ${SYSROOT}/opt/AAC/lib -name "*.so" -exec cp {} ${JNI_DEPLOY} \;