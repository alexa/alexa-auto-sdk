/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_ALEXA_MEDIAPLAYBACKREQUESTORHANDLER_H_
#define SAMPLEAPP_ALEXA_MEDIAPLAYBACKREQUESTORHANDLER_H_

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Alexa/MediaPlaybackRequestor/InvocationReason.h>

#include <chrono>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MediaPlaybackRequestorHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using InvocationReason = aasb::message::alexa::mediaPlaybackRequestor::InvocationReason;

class MediaPlaybackRequestorHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;
    std::chrono::time_point<std::chrono::system_clock> m_startTime{};

protected:
    MediaPlaybackRequestorHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<MediaPlaybackRequestorHandler> {
        return std::shared_ptr<MediaPlaybackRequestorHandler>(new MediaPlaybackRequestorHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    std::weak_ptr<View> m_console{};

    /*
     * Send @c MediaPlaybackRequestor event to the Alexa music cloud requesting resuming the last playing
     * media after the in vehicle infotainment system restarted
     * @param invocationReason Always provide AUTOMOTIVE_STARTUP for the API invocation by the auto resume handler in the IVI
     * @param elapsedBootTime time elapsed from the IVI boot up till the API call in miliseconds.
     */
    void requestMediaPlayback(InvocationReason invocationReason, long long int elapsedBootTime);

    /*
     * Response received from the Alexa engine for the @c requestMediaPlayback event
     */
    void handleMediaPlaybackResponseMessage(const std::string& message);

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_MEDIAPLAYBACKREQUESTORHANDLER_H_
