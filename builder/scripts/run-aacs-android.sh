#!/bin/bash
set -e

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source ${THISDIR}/common.sh

#
# Option
#

usageExit() {
	echo "Usage: gradle [options] [<module-path> ...]"
	echo ""
	echo " -h,--help                  = Print usage information and exit."
	echo ""
	echo " -g,--debug                 = Build with debugging options."
	echo " -c,--clean                 = Do clean."
	echo ""
	exit 1
}

PARAMS="$@"
POSITIONAL=()
EXTRA_CONFS=()
while [[ $# -gt 0 ]]; do
	key="$1"
	case $key in
		-h|--help)
		usageExit
		shift
		;;
		-g|--debug)
		DEBUG_BUILD="1"
		shift
		;;
		-c|--clean)
		CLEAN="1"
		shift
		;;
		-*|--*)
		echo "ERROR: Unknown option '$1'"
		usageExit
		shift
		;;
                *) # Any other additional arguments
                POSITIONAL+=("$1")
                shift
                ;;
	esac
done

set -- "${POSITIONAL[@]}"

# Default values
CLEAN=${CLEAN:-0}
DEBUG_BUILD=${DEBUG_BUILD:-0}

DEPLOY_DIR="${BUILDER_HOME}/deploy"
AACS_DIR="${SDK_HOME}/platforms/android/alexa-auto-client-service"
ANDROID_SERVICE_DIR="${AACS_DIR}/android-service"
ANDROID_SERVICE="${ANDROID_SERVICE_DIR}/service"
ANDROID_SERVICE_LIBS_FOLDER=${ANDROID_SERVICE}/libs
IPC_DIR="${AACS_DIR}/ipc"
CONSTANTS_DIR="${AACS_DIR}/constants"
UTILS_DIR="${AACS_DIR}/commonutils"
TTS_DIR="${AACS_DIR}/tts"
AAR_DEPLOY_DIR="${DEPLOY_DIR}/aar"
APK_DEPLOY_DIR="${DEPLOY_DIR}/apk"

EXTRA_MODULES=$@

if [ $(uname) = "Darwin" ]; then
	SED="gsed"
	FIND="gfind"
else
	SED="sed"
	FIND="find"
fi

run_service_gradle() {
	local gradle_command="assembleLocalRelease"

	pushd ${1}/
	if [ ${CLEAN} = "1" ]; then
		gradle clean
	fi
	if [ ${DEBUG_BUILD} = "1" ]; then
		gradle_command="assembleLocalDebug"
	fi
	gradle ${gradle_command}
	popd
}

run_gradle() {
	local gradle_command="assembleRelease"

	pushd ${1}/
	if [ ${CLEAN} = "1" ]; then
		gradle clean
	fi
	if [ ${DEBUG_BUILD} = "1" ]; then
		gradle_command="assembleDebug"
	fi
	gradle ${gradle_command}
	popd
}

run_aacs_gradle() {
        local gradle_command="assembleRelease"
        if [ ${DEBUG_BUILD} = "1" ]; then
                gradle_command="assembleDebug"
        fi
	pushd ${ANDROID_SERVICE_DIR}/modules/aacs-extra
        if [ ${CLEAN} = "1" ]; then
                gradle clean
        fi
	gradle ${gradle_command}
	popd
	copy_aar ${ANDROID_SERVICE_DIR}/modules
        for module in ${EXTRA_MODULES} ; do
                if [ -d "${module}/aacs/android" ]; then
                        local module_path=$(${FIND} ${module}/aacs/android/modules -mindepth 1 -maxdepth 1 -type d 2> /dev/null)
                        local module_name=$(basename ${module_path})
                        note "Running ${module_name} build"
                        pushd ${module}/aacs/android
                        if [ ${CLEAN} = "1" ]; then
                                gradle clean
                        fi
                        gradle -PaarDir=${AAR_DEPLOY_DIR} ${gradle_command}
                        popd
                        copy_aar "$(realpath ${module})/aacs/android/modules"
                fi
        done
}

copy_aar() {
	local src="${1}/*/build/outputs/aar/*-release.aar"
	if [ ${DEBUG_BUILD} = "1" ]; then
		src="${1}/*/build/outputs/aar/*-debug.aar"
	fi
	if ! ls ${src} 1> /dev/null 2>&1; then
		# Gradle version 5.1+ may not generate -release/-debug AARs
		src="${1}/*/build/outputs/aar/*.aar"
	fi
	for aar in $(${FIND} ${src} 2> /dev/null) ; do
		cp ${aar} ${AAR_DEPLOY_DIR}
	done
}

copy_apk() {
	local src="${1}/build/outputs/apk/local/release/*-release-unsigned.apk"
	if [ ${DEBUG_BUILD} = "1" ]; then
		src="${1}/build/outputs/apk/local/debug/*-debug.apk"
	fi
	cp ${src} ${APK_DEPLOY_DIR}
}


clean_aar() {
	#Clean sample app aars for AACS
	mkdir -p ${AAR_DEPLOY_DIR} && rm -rf ${AAR_DEPLOY_DIR}/sample-*
}

clean_apk() {
	#Clean sample app aars for AACS
	mkdir -p ${APK_DEPLOY_DIR} && rm -rf ${APK_DEPLOY_DIR}/*.apk
}

note "$(head ${DEPLOY_DIR}/buildinfo.txt)"

clean_aar
clean_apk

run_gradle ${IPC_DIR}
copy_aar ${IPC_DIR}
run_gradle ${CONSTANTS_DIR}
copy_aar ${CONSTANTS_DIR}
run_gradle ${UTILS_DIR}
copy_aar ${UTILS_DIR}
run_gradle ${TTS_DIR}
copy_aar ${TTS_DIR}

run_aacs_gradle

if [ -d ${ANDROID_SERVICE_LIBS_FOLDER} ]; then
	rm -f ${ANDROID_SERVICE_LIBS_FOLDER}/*
        cp -r `ls -A ${AAR_DEPLOY_DIR}/* | grep -v "sample-*"` ${ANDROID_SERVICE_LIBS_FOLDER}/ 
else
	mkdir  ${ANDROID_SERVICE_LIBS_FOLDER}/
	cp -r `ls -A ${AAR_DEPLOY_DIR}/* | grep -v "sample-*"` ${ANDROID_SERVICE_LIBS_FOLDER}/	
fi
run_service_gradle ${ANDROID_SERVICE_DIR}
copy_apk  ${ANDROID_SERVICE}
