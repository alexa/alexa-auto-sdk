#!/bin/bash
set -e

#
# ./run-docker.sh <command> <arguments>
#

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_HOME=${THIS_DIR}/../..

VM_HOME="/home/builder"
IMAGE_NAME="aac/ubuntu-base:latest"
VOLUME_NAME="buildervolume"
VOLUME_MOUNT_POINT="/workdir"

execute_command() {
	docker run -it --rm \
	-v ${VOLUME_NAME}:${VOLUME_MOUNT_POINT} \
	-v ${SDK_HOME}:${VM_HOME}/aac \
	-e ANDROID_TOOLCHAIN=${VOLUME_MOUNT_POINT}/android \
	${IMAGE_NAME} $@
}

if [[ "$(docker images -q ${IMAGE_NAME} 2> /dev/null)" == "" ]]; then
	echo "Building Docker image..."
	docker build -t ${IMAGE_NAME} ${THIS_DIR}
fi

if [[ "$(docker volume ls | grep ${VOLUME_NAME} 2> /dev/null)" == "" ]]; then
	echo "Creating Docker volume \"${VOLUME_NAME}\"..."
	docker volume create --name ${VOLUME_NAME}
	echo "Changing permissions for volume..."
	execute_command sudo chown -R builder:builder ${VOLUME_MOUNT_POINT}
fi

echo "Run Docker image..."
execute_command $@