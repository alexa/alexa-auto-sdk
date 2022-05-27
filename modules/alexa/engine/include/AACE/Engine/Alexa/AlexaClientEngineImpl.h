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

#ifndef AACE_ENGINE_ALEXA_ALEXA_CLIENT_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_ALEXA_CLIENT_ENGINE_IMPL_H

#include <ACL/AVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/AVS/DialogUXStateAggregator.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>

#include "AACE/Alexa/AlexaClient.h"

namespace aace {
namespace engine {
namespace alexa {

static const int ENGINE_TYPE_AVS_INT = 1;
static const int ENGINE_TYPE_LOCAL_INT = 2;

class AlexaClientEngineImpl
        : public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface
        , public aace::alexa::AlexaClientEngineInterface {
private:
    AlexaClientEngineImpl(
        std::shared_ptr<aace::alexa::AlexaClient> alexaClientPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> audioFocusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> visualFocusManager);

public:
    static std::shared_ptr<AlexaClientEngineImpl> create(
        std::shared_ptr<aace::alexa::AlexaClient> alexaClientPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> audioFocusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> visualFocusManager);

    // AuthObserverInterface
    void onAuthStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) override;

    // ConnectionStatusObserverInterface
    void onConnectionStatusChanged(
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason)
        override;
    void onConnectionStatusChanged(
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
        const std::vector<
            alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::EngineConnectionStatus>&
            engineStatuses) override;

    // DialogUXStateObserverInterface
    void onDialogUXStateChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState state) override;

    // AlexaClientEngineInterface
    void onStopForegroundActivity() override;

private:
    std::shared_ptr<aace::alexa::AlexaClient> m_alexaClientPlatformInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_audioFocusManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_visualFocusManager;
    alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status m_connectionStatus;

    static inline aace::alexa::AlexaClient::ConnectionType convertConnectionType(int connectionType) {
        switch (connectionType) {
            case ENGINE_TYPE_AVS_INT:
                return aace::alexa::AlexaClient::ConnectionType::AVS;
                break;
            case ENGINE_TYPE_LOCAL_INT:
                return aace::alexa::AlexaClient::ConnectionType::LOCAL;
                break;
            default:
                throw("Unknown connection type.");
        }
    }

    static inline aace::alexa::AlexaClient::ConnectionStatus convertConnectionStatus(
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status connectionStatus) {
        using Status = alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status;

        switch (connectionStatus) {
            case Status::DISCONNECTED:
                return aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED;
                break;
            case Status::PENDING:
                return aace::alexa::AlexaClient::ConnectionStatus::PENDING;
                break;
            case Status::CONNECTED:
                return aace::alexa::AlexaClient::ConnectionStatus::CONNECTED;
                break;
            default:
                return aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED;
                break;
        }
    }

    static inline aace::alexa::AlexaClient::ConnectionChangedReason convertReason(
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason) {
        using ChangedReason =
            alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason;

        switch (reason) {
            case ChangedReason::NONE:
                return aace::alexa::AlexaClient::ConnectionChangedReason::NONE;
                break;
            case ChangedReason::SUCCESS:
                return aace::alexa::AlexaClient::ConnectionChangedReason::SUCCESS;
                break;
            case ChangedReason::UNRECOVERABLE_ERROR:
                return aace::alexa::AlexaClient::ConnectionChangedReason::UNRECOVERABLE_ERROR;
                break;
            case ChangedReason::ACL_CLIENT_REQUEST:
                return aace::alexa::AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST;
                break;
            case ChangedReason::ACL_DISABLED:
                return aace::alexa::AlexaClient::ConnectionChangedReason::ACL_DISABLED;
                break;
            case ChangedReason::DNS_TIMEDOUT:
                return aace::alexa::AlexaClient::ConnectionChangedReason::DNS_TIMEDOUT;
                break;
            case ChangedReason::CONNECTION_TIMEDOUT:
                return aace::alexa::AlexaClient::ConnectionChangedReason::CONNECTION_TIMEDOUT;
                break;
            case ChangedReason::CONNECTION_THROTTLED:
                return aace::alexa::AlexaClient::ConnectionChangedReason::CONNECTION_THROTTLED;
                break;
            case ChangedReason::INVALID_AUTH:
                return aace::alexa::AlexaClient::ConnectionChangedReason::INVALID_AUTH;
                break;
            case ChangedReason::PING_TIMEDOUT:
                return aace::alexa::AlexaClient::ConnectionChangedReason::PING_TIMEDOUT;
                break;
            case ChangedReason::WRITE_TIMEDOUT:
                return aace::alexa::AlexaClient::ConnectionChangedReason::WRITE_TIMEDOUT;
                break;
            case ChangedReason::READ_TIMEDOUT:
                return aace::alexa::AlexaClient::ConnectionChangedReason::READ_TIMEDOUT;
                break;
            case ChangedReason::FAILURE_PROTOCOL_ERROR:
                return aace::alexa::AlexaClient::ConnectionChangedReason::FAILURE_PROTOCOL_ERROR;
                break;
            case ChangedReason::INTERNAL_ERROR:
                return aace::alexa::AlexaClient::ConnectionChangedReason::INTERNAL_ERROR;
                break;
            case ChangedReason::SERVER_INTERNAL_ERROR:
                return aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_INTERNAL_ERROR;
                break;
            case ChangedReason::SERVER_SIDE_DISCONNECT:
                return aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT;
                break;
            case ChangedReason::SERVER_ENDPOINT_CHANGED:
                return aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED;
                break;
            default:
                return aace::alexa::AlexaClient::ConnectionChangedReason::NONE;
                break;
        }
    }
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_ALEXA_CLIENT_ENGINE_IMPL_H
