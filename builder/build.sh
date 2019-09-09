#!/bin/bash
set -e

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export BUILDER_HOME=${THISDIR}

source ${BUILDER_HOME}/scripts/common.sh

# SDK Info
export SDK_HOME=${BUILDER_HOME}/..
export SDK_BASE_VERSION="$(bash -c "${BUILDER_HOME}/scripts/gen-version.sh -b")"
export SDK_VERSION="$(bash -c "${BUILDER_HOME}/scripts/gen-version.sh")"

# Execute command
case ${1} in
	"oe")
		error_and_exit "Command \"oe\" is deprecated, please use build.sh [<platform>|clean] [options]"
		;;
	"bitbake")
		shift
		${BUILDER_HOME}/scripts/run-bitbake.sh $@
		;;
	"docker")
		shift
		${BUILDER_HOME}/scripts/run-docker.sh $@
		;;
	"gradle")
		shift
		${BUILDER_HOME}/scripts/run-gradle.sh $@
		;;
	*)
		${BUILDER_HOME}/scripts/run-builder.sh $@
		;;
esac
