/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
     * Returns the Auto SDK client's contribution to the Local Voice Control configuration.
     * The expected return value is a JSON string formatted as the following:
     * <pre>
     *  {
     *      "LocalSkillService": {
     *          "UnixDomainSocketPath": "/some/path/for/socket"
     *      },
     *      "CarControl": {
     *          "CustomAssetsFilePath": "/some/asset/path/file.json"
     *      }
     *  }
     * </pre>
     *
     * This is invoked by the {@link LocalVoiceControlService} during its startup sequence after the
     * client has registered itself with {@link ILVCService#registerClient}. The properly formatted
     * configuration is required for successful operation of the LVC service.
     *
     * Note: The "CustomAssetsFilePath" member of the "CarControl" node is an optional configuration
     * and should be omitted if the Auto SDK application does not support custom car control assets.
     *
     * @return The Auto SDK client configuration string
     */
    String getConfiguration();

    /**
     * Provides Local Voice Control configuration details, which are an aggregation of individual
     * configurations of LVC components including the Alexa Hybrid Engine, skills, services, and
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
     *          }
     *      }
     *  }
     *
     * </pre>
     *
     * @param configuration The aggregate configuration of LVC components as a JSON string
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