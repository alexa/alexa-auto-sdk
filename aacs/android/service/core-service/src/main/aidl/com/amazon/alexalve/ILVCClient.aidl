/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.amazon.alexalve;

/**
 * The Alexa Auto SDK client application implements {@link ILVCClient} to receive communication from
 * the {@link LocalVoiceControlService}. The client is responsible for providing the Auto SDK portion
 * of the Local Voice Control configuration to the {@link LocalVoiceControlService} and providing
 * the necessary pieces of the aggregate LVC configuration from the {@link ILVCService} to the
 * Auto SDK Engine at configuration time. It also receives "started" and "stopped" lifecycle
 * callbacks from the LVC service.
 */
interface ILVCClient {

    /**
     * Returns the client's contribution to the Local Voice Control configuration.
     * This is invoked by the {@link LocalVoiceControlService} during its startup sequence after the
     * client has registered itself with {@link ILVCService#registerClient}. The properly formatted
     * configuration is required for successful operation of the LVC service, and the expected
     * return value is a JSON string formatted as the following:
     * <pre>
     *  {
     *      "LocalSkillService": {
     *          "UnixDomainSocketPath": "/some/path/for/socket"
     *      },
     *      "CarControl": {
     *          "CustomAssetsFilePath": "/some/asset/path/file.json"
     *      },
     *      "CustomVolume": {
     *          "minVolumeValue": "<Min VUI Volume Value>",
     *          "maxVolumeValue": "<Max VUI Volume Value>",
     *          "volumeAdjustmentStepValue": "<Device volume adjustment step value>"
     *      },
     *      "LocalSearch": {
     *          "NavigationPOISocketDir": "/some/directory/for/sockets",
     *          "NavigationPOISocketName": "poi_navigation.socket",
     *          "POIEERSocketDir": "/some/directory/for/sockets",
     *          "POIEERSocketName": "poi_eer.socket"
     *      },
     *      "ExternalSkillConfigurations": {
     *          "localAlexaApiService": {
     *              "tcpEndpoint": {
     *                  "type": "tcp",
     *                  "details": {
     *                      "address": "<IP address, e.g. 127.0.0.1>",
     *                      "port": "<TCP port, e.g. 9090>"
     *                  }
     *              }
     *          },
     *          "skillCollections": [
     *              {
     *                  "skillCollectionId": "<Name of the skill collection>",
     *                  "type": "<socket or tcp>",
     *                  "details": {
     *                      "socketDirectory": "<Path to the socket directory>",
     *                      "socketPermission": "<Permission of socket, e.g. OWNER or ALL>",
     *                      "socketAddress": "<ip:port, e.g. 127.0.0.1:9000>",
     *                  },
     *                  "artifactBundles": [
     *                      {
     *                          "path": "<path to artifact model bundle>",
     *                          "checksum": "<MD5 checksum of the artifact bundle>"
     *                      }
     *                  ]
     *              }
     *          ]
     *      }
     *  }
     * </pre>
     *
     * The nodes of this configuration are as described below. Optional nodes should be omitted when
     * not relevant to your LVC client:
     *
     * "LocalSkillService" (required):
     *      - "UnixDomainSocketPath" (required): The absolute path where a socket will be created
     *      for communication between the components of the LVC APK and the Local Skill Service of
     *      the Auto SDK Engine.
     *      Use the same path when configuring the Auto SDK Engine's Local Skill Service module.
     * "CarControl" (optional):
     *      - "CustomAssetsFilePath" (optional): Provides the path to a file containing an
     *      additional set of assets to be used to identify car control endpoints, beyond the
     *      defaults already used by Auto SDK. The format of this file must match the expected
     *      assets file schema.
     * "CustomVolume" (optional): Specifies the LVC custom VUI volume range. This should only be
     *      used if your Auto SDK client uses a custom volume range for online utterances. When
     *      this node is present, then all fields are required to be to set to same values as those
     *      configured in the cloud.
     *      - "minVolumeValue" (required): The minimum volume to target by VUI
     *      - "maxVolumeValue" (required): The maximum volume to target by VUI
     *      - "volumeAdjustmentStepValue" (required): The volume increment with respect to
     *         device volume. This refers to the change notified via
     *         @c AlexaSpeaker.speakerSettingsChanged(), which uses the 0-100 range.
     * "LocalSearch" (optional, but required if the Auto SDK Local Navigation module is used):
     *     - "NavigationPOISocketDir" (required): The absolute path to a directory where the
     *       socket will be created for communication between the local navigation skill in
     *       an LVC APK process and local search components running in the Auto SDK Engine.
     *       Use the same directory when configuring the Auto SDK Engine's Local Navigation module.
     *     - "NavigationPOISocketName" (required): The name of the socket used for
     *       communication between the local navigation skill and POI local search components
     *       in the Auto SDK Engine.
     *       Use the same name when configuring the Auto SDK Engine's Local Navigation module.
     *     - "POIEERSocketDir" (required): The absolute path to a directory where the socket
     *       will be created for communication between the offline Alexa service in an LVC APK
     *       process and POI local search components running in the Auto SDK Engine.
     *       Use the same directory when configuring the Auto SDK Engine's Local Navigation module.
     *     - "POIEERSocketName" (required): The name of the socket used for communication
     *       between the offline Alexa service and the POI local search components running in the
     *       the Auto SDK Engine.
     *       Use the same name when configuring the Auto SDK Engine's Local Navigation module.
     * "ExternalSkillConfigurations" (optional): Specifies configurations for additional
     *      local-hosted skills
     *
     * @return The client configuration string
     */
    String getConfiguration();

    /**
     * Provides Local Voice Control configuration details, which are an aggregation of individual
     * configurations of LVC components including the Alexa engine, skills, services, and
     * the Auto SDK client. The Auto SDK client application may retrieve the relevant pieces of
     * this configuration to configure the Auto SDK Engine.
     *
     * The provided configuration is a JSON string with the following format:
     * <pre>
     *  {
     *      "AlexaHybridEngine": {
     *          "ArtifactManager" : {
     *              "IngestionEP" : {
     *                  "UnixDomainSocketPath": "/some/path/to/socket"
     *              }
     *          },
     *          "ExecutionController" : {
     *              "PlatformInterfaceSocketPath": "/some/path/for/socket",
     *              "PlatformInterfaceSocketPermissions": "some-permission",
     *              "MessageRouterSocketPath": "/some/path/for/socket"
     *          },
     *          "PlatformInterfaces" : {
     *              "UnixDomainSocketPath": "/some/path/to/socket",
     *              "UnixDomainSocketPermissions": "some-permission"
     *          }
     *      },
     *      "AACE" : {
     *          "LocalSkillService": {
     *              "UnixDomainSocketPath": "/some/path/for/socket"
     *          },
     *          "CarControl": {
     *              "CustomAssetsFilePath": "/some/asset/path/file.json"
     *          },
     *          "CustomVolume": {
     *              "minVolumeValue": "<Min VUI Volume Value>",
     *              "maxVolumeValue": "<Max VUI Volume Value>",
     *              "volumeAdjustmentStepValue": "<Device volume adjustment step value>"
     *          },
     *          "LocalSearch": {
     *              "NavigationPOISocketDir": "/some/directory/for/sockets",
     *              "NavigationPOISocketName": "poi_navigation.socket",
     *              "POIEERSocketDir": "/some/directory/for/sockets",
     *              "POIEERSocketName": "poi_eer.socket"
     *          }
     *      }
     *  }
     *
     * </pre>
     *
     * @param configuration The aggregate configuration of LVC components as a JSON string.
     *        The "AACE" node is the configuration exactly as provided by your implementation in
     *        {@link ILVCClient#getConfiguration}.
     */
    void configure(String configuration);

    /**
     * Notifies the client that the {@link LocalVoiceControlService} has completed its startup
     * sequence after a request to start via {@link ILVCService#start}. This callback will be
     * invoked after all internal components have started.
     */
    oneway void onStart();

    /**
     * Notifies the client that the {@link LocalVoiceControlService} has completed its shutdown
     * sequence after a request to stop via {@link ILVCService#stop}. This callback will be
     * invoked after all internal components have stopped.
     */
    oneway void onStop();
}
