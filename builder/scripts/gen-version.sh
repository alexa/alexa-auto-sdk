#!/bin/bash

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export BUILDER_HOME=${THISDIR}/..
export SDK_HOME=${BUILDER_HOME}/..

source ${THISDIR}/version

PARAMS="$@"
POSITIONAL=()
while [[ $# -gt 0 ]]; do
	key="$1"
	case $key in
		-l|--local-only)
		PRINT_BASE="0"
		shift
		;;
		-b|--base-only)
		PRINT_LOCAL="0"
		shift
		;;
		-c|--codename)
		printf '%s' ${AAC_CODENAME}
		exit
		;;
		*)
		POSITIONAL+=("$1")
		shift
		;;
	esac
done
set -- "${POSITIONAL[@]}"

# Default values
PRINT_BASE=${PRINT_BASE:-1}
PRINT_LOCAL=${PRINT_LOCAL:-1}

localversion() {
	timestamp=`date +%y%m%d%H%M%S`
	if [ ! -z ${CODEBUILD_SRC_DIR} ]; then
		# SDK is cloned via CodeBuild
		printf '%s%s' -ci -$timestamp
	elif test -z "$(git rev-parse --show-cdup 2>/dev/null)" &&
		head=`git rev-parse --verify --short HEAD 2>/dev/null`; then
		# SDK is managed with the single git repository
		if [ -z "`git describe --tags --exact-match 2>/dev/null`" ]; then
			if atag="`git describe --tags 2>/dev/null`"; then
				echo "$atag" | awk -F- '{printf("-%05d-%s", $(NF-1),$(NF))}'
			else
				printf '%s%s' -g $head
			fi
		fi
		if git diff-index --name-only HEAD | grep -q ".*"; then
			printf '%s' -dirty
		fi
	fi
}

cd ${SDK_HOME}

if [ ${PRINT_BASE} = "1" ]; then
	printf '%s' ${AAC_VERSION}
fi

if [ ${PRINT_LOCAL} = "1" ]; then
	localversion
fi
