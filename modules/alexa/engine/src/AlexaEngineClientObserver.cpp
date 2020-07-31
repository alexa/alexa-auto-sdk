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

#include "AACE/Engine/Alexa/AlexaEngineClientObserver.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

std::shared_ptr<AlexaEngineClientObserver> AlexaEngineClientObserver::create() {
    return std::shared_ptr<AlexaEngineClientObserver>(new AlexaEngineClientObserver());
}

bool AlexaEngineClientObserver::waitFor(
    const alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State authState,
    const std::chrono::seconds duration) {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_trigger.wait_for(lock, duration, [this, authState]() { return authState == m_authState; });
}

bool AlexaEngineClientObserver::waitFor(
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status connectionStatus,
    const std::chrono::seconds duration) {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_trigger.wait_for(
        lock, duration, [this, connectionStatus]() { return connectionStatus == m_connectionStatus; });
}

void AlexaEngineClientObserver::onAuthStateChange(
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state,
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) {
    std::lock_guard<std::mutex> lock{m_mutex};
    m_authState = state;
    m_trigger.notify_all();
}

void AlexaEngineClientObserver::onConnectionStatusChanged(
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connectionStatus = status;
    m_trigger.notify_all();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
