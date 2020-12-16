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

EXTRA_MODULES=$@

DEPLOY_DIR="${BUILDER_HOME}/deploy"
ANDROID_PLATFORM_DIR="${SDK_HOME}/platforms/android"
ANDROID_SAMPLE_DIR="${SDK_HOME}/samples/android"
SYSROOTS_DIR="${ANDROID_PLATFORM_DIR}/native-build"
AAR_DEPLOY_DIR="${DEPLOY_DIR}/aar"

if [ $(uname) = "Darwin" ]; then
	SED="gsed"
	FIND="gfind"
else
	SED="sed"
	FIND="find"
fi

prepare_sysroots() {
	note "Preparing Android sysroots..."

	mkdir -p ${SYSROOTS_DIR} && rm -rf ${SYSROOTS_DIR}/*

	cp ${DEPLOY_DIR}/buildinfo.txt ${SYSROOTS_DIR}

	local targets=$(${FIND} ${DEPLOY_DIR} -print0 |
		${SED} -znE 's|.*/android.*/aac-sdk-build-(.+-android-[0-9]+)\.tar\.gz$|\1|p' |
		tr '\0' '\n')

	if [ -z "${targets}" ]; then
		error "No targets available!"
	fi

	for target in ${targets}; do
		if [ -d ${SYSROOTS_DIR}/${target} ]; then
			warn "Sysroot for ${target} is already exists, skipping..."
			continue
		fi
		mkdir -p ${SYSROOTS_DIR}/${target}
		tar xf ${DEPLOY_DIR}/android*/aac-sdk-build-${target}.tar.gz -C ${SYSROOTS_DIR}/${target}
	done
}

run_gradle() {
	local project_properties=""
	local extra_modules=""
	local gradle_command="assembleRelease"
	for m in ${EXTRA_MODULES} ; do
		local module=$(realpath ${m})
		for module_path in $(${FIND} ${module}/platforms/android/modules -mindepth 1 -maxdepth 1 -type d 2> /dev/null) ; do
			local module_name=$(basename ${module_path})
			extra_modules="${module_name},${extra_modules}"
			project_properties="${project_properties} -Paace.${module_name}.dir=${module_path}"
		done
	done
	project_properties="${project_properties} -Paace.extraModules=${extra_modules}"

	pushd ${ANDROID_PLATFORM_DIR}/modules
	if [ ${CLEAN} = "1" ]; then
		gradle clean
	fi
	if [ ${DEBUG_BUILD} = "1" ]; then
		gradle_command="assembleDebug"
	fi
	gradle ${project_properties} ${gradle_command}
	popd
}

copy_aar() {
	local src="${1}/modules/*/build/outputs/aar/*-release.aar"
	if [ ${DEBUG_BUILD} = "1" ]; then
		src="${1}/modules/*/build/outputs/aar/*-debug.aar"
	fi
	if ! ls ${src} 1> /dev/null 2>&1; then
		# Gradle version 5.1+ may not generate -release/-debug AARs
		src="${1}/modules/*/build/outputs/aar/*.aar"
	fi
	for aar in $(${FIND} ${src} 2> /dev/null) ; do
		cp ${aar} ${AAR_DEPLOY_DIR}
	done
}

clean_aar() {
	mkdir -p ${AAR_DEPLOY_DIR} && rm -rf ${AAR_DEPLOY_DIR}/*
}

run_sample_gradle() {
	local gradle_command="assembleRelease"
	if [ ${DEBUG_BUILD} = "1" ]; then
		gradle_command="assembleDebug"
	fi
	# Sample Core Build
	pushd ${ANDROID_SAMPLE_DIR}/modules/sample-core
	if [ ${CLEAN} = "1" ]; then
		gradle clean
	fi
	gradle ${gradle_command}
	popd
	# Sample APL Build (optional)
	if [ -f ${ANDROID_SAMPLE_DIR}/app/src/main/libs/aplRelease.aar ] || [ -f ${ANDROID_SAMPLE_DIR}/modules/sample-apl/src/main/libs/aplRelease.aar ]; then
		note "Running Sample APL Build"
		pushd ${ANDROID_SAMPLE_DIR}/modules/sample-apl
		if [ ${CLEAN} = "1" ]; then
			gradle clean
		fi
		gradle ${gradle_command}
		popd
	fi
	# Sample Connectivity Build
	note "Running Sample Connectivity Build"
	pushd ${ANDROID_SAMPLE_DIR}/modules/sample-connectivity
	if [ ${CLEAN} = "1" ]; then
		gradle clean
	fi
	gradle ${gradle_command}
	popd
	# Copy the AARs
	copy_aar ${ANDROID_SAMPLE_DIR}
	# Extra Module Builds
	for module in ${EXTRA_MODULES} ; do
		if [ -d "${module}/samples/android" ]; then
			local module_path=$(${FIND} ${module}/samples/android/modules -mindepth 1 -maxdepth 1 -type d 2> /dev/null)
			local module_name=$(basename ${module_path})
			note "Running ${module_name} build"
			pushd ${module}/samples/android
			if [ ${CLEAN} = "1" ]; then
				gradle clean
			fi
			gradle -PaarDir=${AAR_DEPLOY_DIR} ${gradle_command}
			popd
			copy_aar "$(realpath ${module})/samples/android"
		fi
	done
}

if [ ! -e ${DEPLOY_DIR}/buildinfo.txt ]; then
	error "Build is not available!"
fi

note "$(head ${DEPLOY_DIR}/buildinfo.txt)"

prepare_sysroots
run_gradle
clean_aar
copy_aar ${ANDROID_PLATFORM_DIR}
for module in ${EXTRA_MODULES} ; do
	copy_aar "$(realpath ${module})/platforms/android"
done

note "Running Sample Core Build"
run_sample_gradle

