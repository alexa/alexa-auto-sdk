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

#include "SampleApp/Alexa/AudioPlayerHandler.h"

#include "AACE/Engine/Core/EngineMacros.h"

#include <AASB/Message/Alexa/AudioPlayer/GetPlayerDurationMessage.h>
#include <AASB/Message/Alexa/AudioPlayer/GetPlayerPositionMessage.h>

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::audioPlayer;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioPlayerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Default timeout (in milliseconds) to wait for async reply
 */
static const uint32_t ASYNC_REPLY_TIMEOUT = 1000;

AudioPlayerHandler::AudioPlayerHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr) && (m_messageBroker != nullptr));
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> AudioPlayerHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AudioPlayerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void AudioPlayerHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to PlayerActivityChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePlayerActivityChangedMessage(message); },
        PlayerActivityChangedMessage::topic(),
        PlayerActivityChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetPlayerDurationMessageReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetPlayerDurationMessageReply(message); },
        GetPlayerDurationMessageReply::topic(),
        GetPlayerDurationMessageReply::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetPlayerPositionMessageReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetPlayerPositionMessageReply(message); },
        GetPlayerPositionMessageReply::topic(),
        GetPlayerPositionMessageReply::action());
}

void AudioPlayerHandler::handlePlayerActivityChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received PlayerActivityChangedMessage");
    PlayerActivityChangedMessage msg = json::parse(message);
    playerActivityChanged(msg.payload.state);
}

void AudioPlayerHandler::playerActivityChanged(PlayerActivity state) {
    std::stringstream ss;
    ss << "playerActivityChanged:state=" << convertPlayerActivityToString(state);

    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("Player activity changed:", convertPlayerActivityToString(state));
        }
    });
    return;
}

int64_t AudioPlayerHandler::getPlayerDuration() {
    // Publish the "GetPlayerDurationMessage" message
    GetPlayerDurationMessage msg;
    m_messageBroker->publish(msg.toString());

    // get value from reply
    return waitForAsyncReply(msg.header.id);
}

int64_t AudioPlayerHandler::getPlayerPosition() {
    // Publish the "GetPlayerPositionMessage" message
    GetPlayerPositionMessage msg;
    m_messageBroker->publish(msg.toString());

    // get value from reply
    return waitForAsyncReply(msg.header.id);
}

void AudioPlayerHandler::handleGetPlayerDurationMessageReply(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetPlayerDurationMessageReply");
    GetPlayerDurationMessageReply msg = json::parse(message);

    // set the value in the promise
    auto promise = getReplyMessagePromise(msg.header.messageDescription.replyToId);
    ThrowIfNull(promise, "invalidPromise");
    promise->set_value(msg.payload.playbackDuration);
}

void AudioPlayerHandler::handleGetPlayerPositionMessageReply(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetPlayerPositionMessageReply");
    GetPlayerPositionMessageReply msg = json::parse(message);

    // set the value in the promise
    auto promise = getReplyMessagePromise(msg.header.messageDescription.replyToId);
    ThrowIfNull(promise, "invalidPromise");
    promise->set_value(msg.payload.playbackPosition);
}

void AudioPlayerHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AudioPlayerHandler", message);
}

void AudioPlayerHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    activity->registerObserver(Event::onGetPlayerPositionAndDuration, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onGetPlayerPositionAndDuration:");

        auto playerPosition = getPlayerPosition();
        auto playerDuration = getPlayerDuration();

        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printLine(
                    "Player Position: " + std::to_string(playerPosition) + " / " + std::to_string(playerDuration));
            }
        });

        return true;
    });
}

int64_t AudioPlayerHandler::waitForAsyncReply(const std::string& messageId) {
    // create the promise for the audio player reply message to fulfill
    std::shared_ptr<AudioPlayerPromise> promise = std::make_shared<AudioPlayerPromise>();

    // create a future to receive the promised audio player reply message when it is received
    std::shared_future<int64_t> future(promise->get_future());

    int64_t value = TIME_UNKNOWN;
    try {
        addReplyMessagePromise(messageId, promise);
        ThrowIfNot(
            future.wait_for(std::chrono::milliseconds(ASYNC_REPLY_TIMEOUT)) == std::future_status::ready,
            "replyMessageTimeout:id=" + messageId);
        ThrowIfNot(future.valid(), "invalidMessageResponse");
        value = future.get();
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "waitForAsyncReply " + std::string(ex.what()));
        promise->set_exception(std::current_exception());
    }
    removeReplyMessagePromise(messageId);
    return value;
}

void AudioPlayerHandler::addReplyMessagePromise(
    const std::string& messageId,
    std::shared_ptr<AudioPlayerPromise> promise) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        ThrowIf(m_promiseMap.find(messageId) != m_promiseMap.end(), "messageIdAlreadyExists");

        // add the promise to the promise map
        m_promiseMap[messageId] = promise;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "addReplyMessagePromise " + std::string(ex.what()));
    }
}

void AudioPlayerHandler::removeReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        // remove the promise from the promise map
        m_promiseMap.erase(messageId);
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "removeReplyMessagePromise " + std::string(ex.what()));
    }
}

std::shared_ptr<AudioPlayerHandler::AudioPlayerPromise> AudioPlayerHandler::getReplyMessagePromise(
    const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        return it->second;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "getReplyMessagePromise " + std::string(ex.what()));
        return nullptr;
    }
}

std::string AudioPlayerHandler::convertPlayerActivityToString(const PlayerActivity& state) {
    switch (state) {
        case PlayerActivity::IDLE:
            return "IDLE";
        case PlayerActivity::PLAYING:
            return "PLAYING";
        case PlayerActivity::STOPPED:
            return "STOPPED";
        case PlayerActivity::PAUSED:
            return "PAUSED";
        case PlayerActivity::BUFFER_UNDERRUN:
            return "BUFFER_UNDERRUN";
        case PlayerActivity::FINISHED:
            return "FINISHED";
    }
    // Invalid player activity
    log(logger::LoggerHandler::Level::ERROR, "Invalid Player Activity.");
    return "";
}

}  // namespace alexa
}  // namespace sampleApp
