/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Connectivity/AlexaConnectivity.h"

namespace aace {
namespace connectivity {

AlexaConnectivity::~AlexaConnectivity() = default;  // key function

std::string AlexaConnectivity::getIdentifier() {
    return std::string();
}

bool AlexaConnectivity::connectivityStateChange() {
    if (auto m_alexaConnectivityEngineInterface_lock = m_alexaConnectivityEngineInterface.lock()) {
        return m_alexaConnectivityEngineInterface_lock->onConnectivityStateChange();
    }
    return false;
}

void AlexaConnectivity::sendConnectivityEvent(const std::string& event, const std::string& token) {
    if (auto m_alexaConnectivityEngineInterface_lock = m_alexaConnectivityEngineInterface.lock()) {
        m_alexaConnectivityEngineInterface_lock->onSendConnectivityEvent(event, token);
    }
}

void AlexaConnectivity::setEngineInterface(
    std::shared_ptr<AlexaConnectivityEngineInterface> alexaConnectivityEngineInterface) {
    m_alexaConnectivityEngineInterface = alexaConnectivityEngineInterface;
}

}  // namespace connectivity
}  // namespace aace
