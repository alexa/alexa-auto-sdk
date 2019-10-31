#!/bin/sh

echo "#####################################"
echo "USAGE: ./${me} Host/Target/Local [Windows User ID] [Stable / Commit Hash]"
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

cd ${THISDIR}

if [ $1 = ${HOST} ]; then
    SERVER_DEST=${SERVER}/${HOST}
    echo "Get ${RELEASE_FILE} from ${SERVER_DEST}"
elif [ $1 = ${TARGET} ]; then
    echo ""
    echo "TODO Release AIDaemonAAC to ${TARGET}"
    echo ""

elif [ $1 = ${LOCAL} ]; then
    echo "${RELEASE_FILE} should be in ${THISDIR}"
else
    echo "#####################################"
    echo "Unknown ${1}"
    echo "#####################################"
    exit 1
fi

if [ $1 = ${HOST} ] || [ $1 = ${TARGET} ]; then
    if test "$#" -gt 2; then
        if [ $3 = "Stable" ]; then
            eval "smbclient --user=$2 '//$KRSEFS01' -c 'cd $SERVER_DEST/Stable/Stable; get $RELEASE_FILE'"
        else
            eval "smbclient --user=$2 '//$KRSEFS01' -c 'cd $SERVER_DEST/Stable/$3; get $RELEASE_FILE'"
        fi
    else
        eval "smbclient --user=$2 '//$KRSEFS01' -c 'cd $SERVER_DEST/Last_Release; get $RELEASE_FILE'"
    fi
fi

if [ $1 = ${HOST} ] || [ $1 = ${LOCAL} ]; then
    echo "#####################################"
    echo "TODO : remove old files"
    echo "#####################################"
else
    echo "TODO : copy files to target"
fi

echo "#####################################"
echo "extract latest file"
echo "#####################################"
tar -xzvf ${THISDIR}/${RELEASE_FILE}


