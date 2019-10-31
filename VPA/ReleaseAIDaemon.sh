#!/bin/sh

echo "#####################################"
echo "USAGE: ./${me} Host/Target/Local [Windows User ID] [Stable]"
echo "#####################################"
echo ""

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

KRSEFS01=10.28.2.24/Public
RELEASE_FILE=aidaemonAAC.tar.gz
SERVER=Dev5_shared/Nissan-VPA/AIDaemonAAC
SERVER_DEST=${SERVER}
TARGET=Target
HOST=Host
LOCAL=Local

rm  -rf ${THISDIR}/${RELEASE_FILE}

if [ $1 = ${HOST} ] || [ $1 = ${LOCAL} ]; then
    SDK_BUILD=${THISDIR}/../builder/deploy/native
    PACKAGE_HOST=${THISDIR}/${HOST}
    SERVER_DEST=${SERVER}/${HOST}

    if [ $1 = ${HOST} ]; then
        echo ""
        echo "Release AIDaemonAAC to ${SERVER_DEST}"
        echo ""
    else
        echo ""
        echo "Release AIDaemonAAC to Simulator"
        echo ""
    fi

    rm -rf ${PACKAGE_HOST}/opt
    rm -rf ${PACKAGE_HOST}/aac-sdk-build-native.tar.gz
    cp ${SDK_BUILD}/aac-sdk-build-native.tar.gz ${PACKAGE_HOST}
    cd ${PACKAGE_HOST}
    tar -xzvf ./aac-sdk-build-native.tar.gz
    cd ${THISDIR}

    tar -czvf ${RELEASE_FILE} -C ${PACKAGE_HOST} . --exclude ./aac-sdk-build-native.tar.gz --exclude ./.DS_Store
else #target
    echo ""
    echo "TODO Release AIDaemonAAC to ${TARGET}"
    echo ""
fi

if [ $1 = ${LOCAL} ]; then
    ssh -p 2222 oad@192.168.56.1 "mkdir -p /home/oad/AIDaemon" && scp -P 2222 ${RELEASE_FILE} oad@192.168.56.1:/home/oad/AIDaemon
    exit 1
fi

if test "$#" -gt 1; then
    eval "smbclient --user=$2 '//$KRSEFS01' -c 'cd $SERVER_DEST/Last_Release; put $RELEASE_FILE'"
fi

if test "$#" -gt 2; then
    if [ $3 = "Stable" ]; then
        AI_DAEMON_VERSION="`python ${THISDIR}/Version.py`"
        eval "smbclient --user=$2 '//$KRSEFS01' -c 'cd $SERVER_DEST/Stable; rmdir $AI_DAEMON_VERSION'"
        eval "smbclient --user=$2 '//$KRSEFS01' -c 'cd $SERVER_DEST/Stable; mkdir $AI_DAEMON_VERSION'"
        eval "smbclient --user=$2 '//$KRSEFS01' -c 'cd $SERVER_DEST/Stable/$AI_DAEMON_VERSION; put $RELEASE_FILE'"
        eval "smbclient --user=$2 '//$KRSEFS01' -c 'cd $SERVER_DEST/Stable/Stable; put $RELEASE_FILE'"
    else
        echo "Invalid arguments => $3"
    fi
fi
