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

#ifndef AACE_ENGINE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_REST_AGENT_H
#define AACE_ENGINE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_REST_AGENT_H

#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/Utils/DeviceInfo.h>

#include <AACE/Engine/Alexa/AlexaEndpointInterface.h>

namespace aace {
namespace engine {
namespace phoneCallController {

/// Struct to represent the account info.
struct AlexaAccountInfo {
    /// The directedId of account.
    std::string directedId;

    /// Enum representing the provisioning status of account.
    enum class AccountProvisionStatus {
        /// Invalid state
        INVALID,

        /// Status is new or unknown.
        UNKNOWN,

        /// Account is provisioned.
        PROVISIONED,

        /// Account is deprovisioned.
        DEPROVISIONED,

        /// Account is auto provisioned.
        AUTO_PROVISIONED,
    };

    /// The provisioning status of the account.
    AccountProvisionStatus provisionStatus;

    /// Constructor
    AlexaAccountInfo() : directedId(""), provisionStatus(AccountProvisionStatus::INVALID) {
    }
};

/**
 * Function to get the account info.
 * 
 * @param authDelegate The reference to @c AuthDelegateInterface to get the auth token.
 * @param deviceInfo The reference to @c DeviceInfo to get the auth token.
 * @return On successful it returns @c AlexaAccountInfo otherwise if will return the default @c AlexaAccountInfo.
 */
AlexaAccountInfo getAlexaAccountInfo(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints);

/**
 * Function to perform the auto provisioning of the account.
 * 
 * @param alexaAccountInfo The reference to AlexaAccountInfo providing the directedId.
 * @param authDelegate The reference to @c AuthDelegateInterface to get the auth token.
 * @param deviceInfo The reference to @c DeviceInfo to get the auth token.
 * @return On successful it returns @c true otherwise if will return the default @c false.
 */
bool doAccountAutoProvision(
    const AlexaAccountInfo& alexaAccountInfo,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints);

}  // namespace phoneCallController
}  // namespace engine
}  // namespace aace

#endif  //AACE_ENGINE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_REST_AGENT_H
