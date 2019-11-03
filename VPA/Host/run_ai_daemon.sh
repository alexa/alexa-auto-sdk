#!/bin/sh

DIR=`pwd`

AI_DAEMON_CONFIG_JSON=${DIR}/configAIDaemon.json
ALEXA_CONFIG_JSON=${DIR}/config.json
ALEXA_MENU_JSON=${DIR}/menu.json
DIR_AVS_DB=${DIR}/AAC-DB
DEVICESERIALNUMBER=`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 10 | head -n 1`
CLIENTID="amzn1.application-oa2-client.0b485e8c035d4c64ae67bab79777ebec"
PRODUCTID="Nissan_VPA_POC"
NISSAN_LOCALE="en-US"
CA_PATH=${DIR}/opt/AAC/etc/certs
TARGET_APP_NAME=${DIR}/opt/AAC/bin/SampleApp

pkill ${TARGET_APP_NAME}
# TODO Remove
pkill AIDaemon

if [ ! -e "${AI_DAEMON_CONFIG_JSON}" ]; then
  cp "${ALEXA_CONFIG_JSON}" "${AI_DAEMON_CONFIG_JSON}"
  rm -rf "$DIR_AVS_DB"
fi

sed -i 's/"deviceSerialNumber": ""/"deviceSerialNumber":"'${DEVICESERIALNUMBER}'"/' ${AI_DAEMON_CONFIG_JSON}
sed -i 's/"clientId": ""/"clientId":"'${CLIENTID}'"/' ${AI_DAEMON_CONFIG_JSON}
sed -i 's/"productId": ""/"productId":"'${PRODUCTID}'"/' ${AI_DAEMON_CONFIG_JSON}

if [ ! -d "$DIR_AVS_DB" ]; then
  mkdir "$DIR_AVS_DB"
fi

sed -i 's|"CURLOPT_CAPATH": ""|"CURLOPT_CAPATH":"'${CA_PATH}'"|' ${AI_DAEMON_CONFIG_JSON}
sed -i 's|"databaseFilePath1": ""|"databaseFilePath":"'${DIR_AVS_DB}'/miscDatabase.db"|' ${AI_DAEMON_CONFIG_JSON}
sed -i 's|"databaseFilePath2": ""|"databaseFilePath":"'${DIR_AVS_DB}'/certifiedsender.db"|' ${AI_DAEMON_CONFIG_JSON}
sed -i 's|"databaseFilePath3": ""|"databaseFilePath":"'${DIR_AVS_DB}'/alertsCapabilityAgent.db"|' ${AI_DAEMON_CONFIG_JSON}
sed -i 's|"databaseFilePath4": ""|"databaseFilePath":"'${DIR_AVS_DB}'/notifications.db"|' ${AI_DAEMON_CONFIG_JSON}
sed -i 's|"databaseFilePath5": ""|"databaseFilePath":"'${DIR_AVS_DB}'/settings.db"|' ${AI_DAEMON_CONFIG_JSON}
sed -i 's|"locale": ""|"locale":"'${NISSAN_LOCALE}'"|' ${AI_DAEMON_CONFIG_JSON}
sed -i 's|"localStoragePath": ""|"localStoragePath":"'${DIR_AVS_DB}'/aace-storage.db"|' ${AI_DAEMON_CONFIG_JSON}

export LD_LIBRARY_PATH=${DIR}/opt/AAC/lib:${LD_LIBRARY_PATH}
echo "${LD_LIBRARY_PATH}"

${TARGET_APP_NAME} --config ${AI_DAEMON_CONFIG_JSON} --level VERBOSE --menu ${ALEXA_MENU_JSON}
