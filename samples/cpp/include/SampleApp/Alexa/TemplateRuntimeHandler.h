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

#ifndef SAMPLEAPP_ALEXA_TEMPLATERUNTIMEHANDLER_H
#define SAMPLEAPP_ALEXA_TEMPLATERUNTIMEHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Alexa/TemplateRuntime/RenderPlayerInfoMessage.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TemplateRuntimeHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class TemplateRuntimeHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    TemplateRuntimeHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<TemplateRuntimeHandler> {
        return std::shared_ptr<TemplateRuntimeHandler>(new TemplateRuntimeHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the ClearPlayerInfo messages received from the Engine.
     * This notifies the platform implementation to dismiss the player info display card.
     *
     * @param [in] message The message received from the Engine
     */
    void handleClearPlayerInfoMessage(const std::string& message);

    /**
     * Handles the ClearTemplate messages received from the Engine.
     * This notifies the platform implementation to dismiss the template display card.
     *
     * @param [in] message The message received from the Engine
     */
    void handleClearTemplateMessage(const std::string& message);

    /**
     * Handles the RenderPlayerInfo messages received from the Engine.
     * This provides visual metadata associated with a user request to Alexa for audio playback.
     *
     * @param [in] message The message received from the Engine
     */
    void handleRenderPlayerInfoMessage(const std::string& message);

    /**
     * Handles the RenderTemplate messages received from the Engine.
     * This provides visual metadata associated with a user request to Alexa.
     *
     * @param [in] message The message received from the Engine
     */
    void handleRenderTemplateMessage(const std::string& message);

    /**
     * Provides the implementation to dismiss the player info display card
     */
    void clearPlayerInfo();

    /**
     * Provides the implementation to dismiss the template display card.
     */
    void clearTemplate();

    /**
     * Provides the implementation to render player info.
     *
     * @param [in] payload Renderable player info metadata in structured JSON format
     * @param [in] audioPlayerState The state of the @c AudioPlayer
     * @param [in] offset The offset in millisecond of the media that @c AudioPlayer is handling
     * @param [in] focusState The @c FocusState of the channel used by TemplateRuntime interface
     */
    void renderPlayerInfo(
        const std::string& payload,
        aasb::message::alexa::templateRuntime::PlayerActivity audioPlayerState,
        std::chrono::milliseconds offset,
        aasb::message::alexa::templateRuntime::FocusState focusState);

    /**
     * Provides the implementation to render a display card for the user.
     *
     * @param [in] payload Renderable template metadata in structured JSON format
     * @param [in] focusState The @c FocusState of the channel used by TemplateRuntime interface
     */
    void renderTemplate(const std::string& payload, aasb::message::alexa::templateRuntime::FocusState focusState);

private:
    std::weak_ptr<View> m_console{};
    std::chrono::time_point<std::chrono::system_clock> m_startPlayerInfo{};
    std::chrono::time_point<std::chrono::system_clock> m_startTemplate{};

    std::chrono::time_point<std::chrono::steady_clock> m_whenCachedLastpayload{};
    std::string m_lastPayload;

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto wasPayloadJustSeen(const std::string& payload) -> bool;
    auto subscribeToAASBMessages() -> void;
    auto convertFocusStateToString(aasb::message::alexa::templateRuntime::FocusState focusState) -> std::string;
    auto convertPlayerActivityToString(aasb::message::alexa::templateRuntime::PlayerActivity activity) -> std::string;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_TEMPLATERUNTIMEHANDLER_H
