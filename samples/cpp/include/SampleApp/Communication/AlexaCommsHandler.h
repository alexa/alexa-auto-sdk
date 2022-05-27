/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_COMMUNICATIONS_COMMUNICATIONS_HANDLER_H
#define SAMPLEAPP_COMMUNICATIONS_COMMUNICATIONS_HANDLER_H

#include "SampleApp/Extension.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Communication/AlexaComms/CallState.h>

namespace sampleApp {
namespace communication {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaCommsHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////
class AlexaCommsHandler : public extension::Extension {
    // Enable extension for dynamic loading
    ENABLE_EXTENSION;

private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

public:
    AlexaCommsHandler(const std::string& name) : Extension(name){};
    virtual ~AlexaCommsHandler() = default;

    /// @name @c Extension functions
    /// @{
    bool initialize(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker) override;
    bool validate(const std::vector<nlohmann::json>& configs) override;
    /// @}

    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles a CallDisplayInfoMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleCallDisplayInfoMessage(const std::string& message);

    /**
     * Handles a CallStateChangedMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleCallStateChangedMessage(const std::string& message);

    /**
     * Publishes AcceptCallMessage to notify the Engine of a platform request to accept an incoming
     * call.
     */
    void acceptCall();

    /**
     * Publishes StopCallMessage to notify the Engine of a platform request to stop the call.
     */
    void stopCall();

    /**
     * Provides the implementation for handling a call display info change.
     *
     * @param [in] displayInfo JSON string with information to be used for display 
     */
    void callDisplayInfo(const std::string& displayInfo);

    /**
     * Provides the implementation to handle a change in call state.
     *
     * @param [in] state The new call state.
     */
    void callStateChanged(aasb::message::communication::alexaComms::CallState state);

    /**
     * Helper method to display the state and display info.
     */
    auto showDisplayInfo() -> void;
    auto showState() -> void;
    auto callStateToString(aasb::message::communication::alexaComms::CallState state) -> std::string;

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;

    /// Call display info
    std::string m_callDisplayInfo;

    /// Call state
    aasb::message::communication::alexaComms::CallState m_callState;
};

}  // namespace communication
}  // namespace sampleApp

#endif  // SAMPLEAPP_COMMUNICATIONS_COMMUNICATIONS_HANDLER_H
