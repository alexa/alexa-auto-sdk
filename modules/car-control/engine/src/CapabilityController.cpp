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

#include <AACE/Engine/CarControl/CapabilityController.h>

namespace aace {
namespace engine {
namespace carControl {

CapabilityController::CapabilityController(const std::string& endpointId, const std::string& interface) :
        // alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG), //TODO
        m_endpointId(endpointId),
        m_interface(interface) {
}

CapabilityController::~CapabilityController() {
}

std::string CapabilityController::getEndpointId() {
    return m_endpointId;
}

std::string CapabilityController::getId() {
    return getInterface();
}

std::string CapabilityController::getInterface() {
    return m_interface;
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
