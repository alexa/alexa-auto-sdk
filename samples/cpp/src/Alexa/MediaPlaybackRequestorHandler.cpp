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

#include "SampleApp/Alexa/MediaPlaybackRequestorHandler.h"

#include <AASB/Message/Alexa/MediaPlaybackRequestor/MediaPlaybackResponseMessage.h>
#include <AASB/Message/Alexa/MediaPlaybackRequestor/RequestMediaPlaybackMessage.h>

// C++ Standard Library
#include <sstream>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MediaPlaybackRequestorHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// alias
using MessageBroker = aace::core::MessageBroker;

using namespace aasb::message::alexa::mediaPlaybackRequestor;

MediaPlaybackRequestorHandler::MediaPlaybackRequestorHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    m_startTime = std::chrono::system_clock::now();
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> MediaPlaybackRequestorHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> MediaPlaybackRequestorHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void MediaPlaybackRequestorHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to MediaPlaybackResponse Messages for RequestMediaPlayback");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleMediaPlaybackResponseMessage(message); },
        MediaPlaybackResponseMessage::topic(),
        MediaPlaybackResponseMessage::action());
}

void MediaPlaybackRequestorHandler::handleMediaPlaybackResponseMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received MediaPlaybackResponseMessage");
}

void MediaPlaybackRequestorHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "MediaPlaybackRequestorHandler", message);
}

void MediaPlaybackRequestorHandler::requestMediaPlayback(
    InvocationReason invocationReason,
    long long int elapsedBootTime) {
    RequestMediaPlaybackMessage requestMediaPlaybackMessage;
    requestMediaPlaybackMessage.payload.invocationReason = invocationReason;
    requestMediaPlaybackMessage.payload.elapsedBootTime = elapsedBootTime;
    m_messageBroker->publish(requestMediaPlaybackMessage.toString());
}

void MediaPlaybackRequestorHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // Media playback requested
    activity->registerObserver(Event::onMediaPlaybackRequested, [=](const std::string&) {
        // In this C++ sample app, this feature is demonstrated with respect to app start start time to API calling current time.
        // This is not the way this feature is expected to be implemented in the car. Since Alexa auto SDK sample app is compatible with various
        // platforms, this example is demonstrated in such a way. OEMs are expected to get the elapsed boot time from their platform specific
        // mechanism and provide the correct value.
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_startTime);
        requestMediaPlayback(InvocationReason::AUTOMOTIVE_STARTUP, elapsed.count());
        log(logger::LoggerHandler::Level::VERBOSE,
            "onMediaPlaybackRequested >> elapsedTime=" + std::to_string(elapsed.count()));
        return true;
    });
}

}  // namespace alexa
}  // namespace sampleApp
