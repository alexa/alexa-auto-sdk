#!/bin/bash

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export BUILDER_HOME=${THISDIR}

source ${BUILDER_HOME}/scripts/common.sh

# SDK Info
export SDK_HOME=${BUILDER_HOME}/..
export SDK_BASE_VERSION="$(bash -c "${BUILDER_HOME}/scripts/gen-version.sh -b")"
export SDK_VERSION="$(bash -c "${BUILDER_HOME}/scripts/gen-version.sh")"

#
# Option
#

usageExit() {
	echo "Usage: $0 <command> [options]"
	exit 1
}

COMMAND=${1}
shift

# Execute command
case ${COMMAND} in
	"help")
		usageExit
		;;
	"oe")
		${BUILDER_HOME}/scripts/run-bitbake.sh $@
		;;
	"cmake")
		${BUILDER_HOME}/scripts/run-cmake.sh $@
		;;
	"docker")
		${BUILDER_HOME}/scripts/run-docker.sh $@
		;;
	*)
		error "Unknown command: ${COMMAND}"
		;;
esac
