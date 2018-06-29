#!/bin/bash

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export BUILDER_HOME=${THISDIR}
export SDK_HOME=${BUILDER_HOME}/..

source ${BUILDER_HOME}/scripts/common.sh

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
