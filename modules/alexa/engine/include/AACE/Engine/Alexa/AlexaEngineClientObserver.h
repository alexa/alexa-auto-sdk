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

#ifndef AACE_ENGINE_ALEXA_ALEXA_ENGINE_CLIENT_OBSERVER_H
#define AACE_ENGINE_ALEXA_ALEXA_ENGINE_CLIENT_OBSERVER_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>

namespace aace {
namespace engine {
namespace alexa {

class AlexaEngineClientObserver
        : public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface {
private:
    AlexaEngineClientObserver() = default;

public:
    static std::shared_ptr<AlexaEngineClientObserver> create();

    bool waitFor(
        const alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State authState,
        const std::chrono::seconds duration = std::chrono::seconds(20));
    bool waitFor(
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status connectionStatus,
        const std::chrono::seconds duration = std::chrono::seconds(20));

    // AuthObserverInterface
    void onAuthStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) override;

    // ConnectionStatusObserverInterface
    void onConnectionStatusChanged(
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason)
        override;

private:
    /// Internal mutex to serialize access to m_connectionStatus and m_authState states.
    std::mutex m_mutex;

    /// A condition variable used to wait for state changes.
    std::condition_variable m_trigger;

    /// The current authorization state.
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State m_authState;

    /// The current connection state.
    alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status m_connectionStatus;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_ALEXA_ENGINE_CLIENT_OBSERVER_H
