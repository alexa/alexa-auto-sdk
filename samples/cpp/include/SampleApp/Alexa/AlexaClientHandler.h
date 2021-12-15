/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_ALEXA_ALEXACLIENTHANDLER_H
#define SAMPLEAPP_ALEXA_ALEXACLIENTHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Alexa/AlexaClient/DialogStateChangedMessage.h>
#include <AASB/Message/Alexa/AlexaClient/AuthStateChangedMessage.h>
#include <AASB/Message/Alexa/AlexaClient/ConnectionStatusChangedMessage.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaClientHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AlexaClientHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    AlexaClientHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AlexaClientHandler> {
        return std::shared_ptr<AlexaClientHandler>(new AlexaClientHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the DialogStateChanged message received from the Engine.
     *
     * @param [in] message The message received from the Engine with the changed state.
     */
    void handleDialogStateChangedMessage(const std::string& message);

    /**
     * Handles the AuthStateChanged message received from the Engine.
     *
     * @param [in] message The message received from the Engine with the AVS authorization state change.
     */
    void handleAuthStateChangedMessage(const std::string& message);

    /**
     * Handles the ConnectionStatusChanged message received from the Engine.
     *
     * @param [in] message The message received from the Engine with the AVS connection status change and the reason.
     */
    void handleConnectionStatusChangedMessage(const std::string& message);

    /**
     * Publishes the StopForegroundActivityMessage to notify the Engine of a platform request to stop the foreground activity, if there is one
     */
    void stopForegroundActivity();

    /**
     * Provides the platform implementation of an AVS authorization state change
     *
     * @param [in] state The new authorization state
     * @param [in] error The error state of the authorization attempt
     */
    void dialogStateChanged(aasb::message::alexa::alexaClient::DialogState state);

    /**
     * Provides the platform implementation of an AVS authorization state change
     *
     * @param [in] state The new authorization state
     * @param [in] error The error state of the authorization attempt
     */
    void authStateChanged(
        aasb::message::alexa::alexaClient::AuthState state,
        aasb::message::alexa::alexaClient::AuthError error);

    /**
     * Provides the platform implementation of an AVS connection status change
     *
     * @param [in] status The new AVS connection status
     * @param [in] reason The reason for the AVS connection status change
     */
    void connectionStatusChanged(
        aasb::message::alexa::alexaClient::ConnectionStatus status,
        aasb::message::alexa::alexaClient::ConnectionChangedReason reason);

private:
    std::weak_ptr<View> m_console{};
    std::weak_ptr<View> m_authStateView{};
    std::weak_ptr<View> m_connectionStatusView{};
    std::weak_ptr<View> m_dialogStateView{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
    auto convertDialogStateToString(const aasb::message::alexa::alexaClient::DialogState& state) -> std::string;
    auto convertAuthStateToString(const aasb::message::alexa::alexaClient::AuthState& state) -> std::string;
    auto convertAuthErrorToString(const aasb::message::alexa::alexaClient::AuthError& error) -> std::string;
    auto convertConnectionStatusToString(const aasb::message::alexa::alexaClient::ConnectionStatus& state)
        -> std::string;
    auto convertConnectionChangedReasonToString(const aasb::message::alexa::alexaClient::ConnectionChangedReason& state)
        -> std::string;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_ALEXACLIENTHANDLER_H
