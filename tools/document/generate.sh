#!/bin/bash
set -e

DOCUMENT_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_HOME=${DOCUMENT_HOME}/../..

#
# Option
#

usageExit() {
	echo "Usage: $0 [options]"
	echo ""
	echo " -o  Output directory"
	echo ""
	exit 1
}

check_doxygen_version() {
	if [ -z $(which doxygen) ]; then
		echo "Doxygen is not installed or not in the path!"
		exit 1
	fi
	DOXYGEN_VERSION=$(doxygen --version)
	RECOMMENDED_VERSION="1.8.14"
	if [ ${DOXYGEN_VERSION} != ${RECOMMENDED_VERSION} ]; then
		echo "*** Doxygen version $(doxygen --version) detected"
		echo "*** Recommended version is ${RECOMMENDED_VERSION}"
	fi
}

check_graphviz_dot() {
	if [ -z $(which dot) ]; then
		echo "Graphviz dot is not installed or not in the path!"
		exit 1
	fi
}

while getopts ":o:" opt; do
	case $opt in
		o ) OUTPUT="$OPTARG";;
		\?) usageExit;;
	esac
done
shift $((OPTIND-1))

# Default values
OUTPUT=${OUTPUT:-"${SDK_HOME}/docs"}

TARGETS=${@:-"android"} # android cpp

check_doxygen_version

for target in $TARGETS; do
	export DOXYGEN_MAINPAGE="${SDK_HOME}/platforms/${target}/README.md"
	export DOXYGEN_OPTIMIZE_OUTPUT_JAVA="NO"
	export DOXYGEN_TARGET_SDK="SDK"
	export EXCLUDE=""
	case ${target} in
		android)
			export DOXYGEN_FILE_PATTERNS="*.java *.md"
			export DOXYGEN_IMAGE_PATH="\
				${SDK_HOME}/platforms/${target}/assets"
			export DOXYGEN_INPUT="\
				${DOXYGEN_MAINPAGE} \
				${SDK_HOME}/platforms/${target}/aace/src/main/java"
			export DOXYGEN_OPTIMIZE_OUTPUT_JAVA="YES"
			export DOXYGEN_TARGET_SDK="SDK (Android)"
			;;
		cpp)
			export DOXYGEN_FILE_PATTERNS="*.cpp *.h *.md"
			export DOXYGEN_IMAGE_PATH="\
				${SDK_HOME}/assets"
			export DOXYGEN_INPUT="\
				${SDK_HOME}/assets \
				${SDK_HOME}/modules \
				${SDK_HOME}/README.md \
				${SDK_HOME}/samples/cpp"
			export DOXYGEN_MAINPAGE="${SDK_HOME}/README.md"
			;;
		*)
			echo "Unknown target: ${target}"
			exit 1
	esac
	export DOXYGEN_EXCLUDE_PATTERNS="${DOXYGEN_MAINPAGE} ${EXCLUDE}"
	export DOXYGEN_IMAGE_PATH="${DOXYGEN_IMAGE_PATH} assets"
	export DOXYGEN_OUTPUT="$(realpath ${OUTPUT})"
	export DOXYGEN_TARGET="${target}"

	cd ${DOCUMENT_HOME} && doxygen doxygen.cfg
	rm ${OUTPUT}/${target}/dir_* # cleanup useless files

	cat > ${DOXYGEN_OUTPUT}/${target}.html <<EOF
<meta http-equiv="REFRESH" content="0;URL=./${target}/index.html">

EOF
done
