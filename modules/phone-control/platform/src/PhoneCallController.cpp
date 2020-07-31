/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/PhoneCallController/PhoneCallController.h"

namespace aace {
namespace phoneCallController {

void PhoneCallController::connectionStateChanged(ConnectionState state) {
    if (m_phoneCallControllerEngineInterface != nullptr) {
        m_phoneCallControllerEngineInterface->onConnectionStateChanged(state);
    }
}

void PhoneCallController::callStateChanged(CallState state, const std::string& callId, const std::string& callerId) {
    if (m_phoneCallControllerEngineInterface != nullptr) {
        m_phoneCallControllerEngineInterface->onCallStateChanged(state, callId, callerId);
    }
}

void PhoneCallController::callFailed(const std::string& callId, CallError code, const std::string& message) {
    if (m_phoneCallControllerEngineInterface != nullptr) {
        m_phoneCallControllerEngineInterface->onCallFailed(callId, code, message);
    }
}

void PhoneCallController::callerIdReceived(const std::string& callId, const std::string& callerId) {
    if (m_phoneCallControllerEngineInterface != nullptr) {
        m_phoneCallControllerEngineInterface->onCallerIdReceived(callId, callerId);
    }
}

void PhoneCallController::sendDTMFSucceeded(const std::string& callId) {
    if (m_phoneCallControllerEngineInterface != nullptr) {
        m_phoneCallControllerEngineInterface->onSendDTMFSucceeded(callId);
    }
}
void PhoneCallController::sendDTMFFailed(const std::string& callId, DTMFError code, const std::string& message) {
    if (m_phoneCallControllerEngineInterface != nullptr) {
        m_phoneCallControllerEngineInterface->onSendDTMFFailed(callId, code, message);
    }
}

void PhoneCallController::deviceConfigurationUpdated(
    std::unordered_map<CallingDeviceConfigurationProperty, bool> configurationMap) {
    if (m_phoneCallControllerEngineInterface != nullptr) {
        m_phoneCallControllerEngineInterface->onDeviceConfigurationUpdated(configurationMap);
    }
}

std::string PhoneCallController::createCallId() {
    if (m_phoneCallControllerEngineInterface != nullptr) {
        return m_phoneCallControllerEngineInterface->onCreateCallId();
    }
    return "";
}

void PhoneCallController::setEngineInterface(
    std::shared_ptr<PhoneCallControllerEngineInterface> phoneCallControllerEngineInterface) {
    m_phoneCallControllerEngineInterface = phoneCallControllerEngineInterface;
}

PhoneCallController::~PhoneCallController() = default;  //key function

}  // namespace phoneCallController
}  // namespace aace
