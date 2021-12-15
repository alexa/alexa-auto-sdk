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

#ifndef SAMPLEAPP_ALEXA_AUDIOPLAYERHANDLER_H
#define SAMPLEAPP_ALEXA_AUDIOPLAYERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Alexa/AudioPlayer/PlayerActivityChangedMessage.h>

#include <unordered_map>
#include <future>
#include <utility>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioPlayerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AudioPlayerHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    AudioPlayerHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AudioPlayerHandler> {
        return std::shared_ptr<AudioPlayerHandler>(new AudioPlayerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    /**
     * Used when audio time is unknown or indeterminate.
     */
    static const int64_t TIME_UNKNOWN = -1;

    /**
     * Handles the PlayerActivityChanged message received from the Engine.
     *
     * @param [in] message The message received from the Engine with the changed audio plyaback state.
     */
    void handlePlayerActivityChangedMessage(const std::string& message);

    /**
     * Handles the GetPlayerDurationMessageReply  received from the Engine.
     *
     * @param [in] message The Reply message with the playback duration, received from the Engine in response to the GetPlayerDuration message sent to the Engine.
     */
    void handleGetPlayerDurationMessageReply(const std::string& message);

    /**
     * Handles the GetPlayerPositionMessageReply  received from the Engine.
     *
     * @param [in] message The Reply message with the playback position, received from the Engine in response to the GetPlayerPosition message sent to the Engine.
     */
    void handleGetPlayerPositionMessageReply(const std::string& message);

    /**
     * Publishes GetPlayerDuration message to notify the Engine to send the playback duration of the audio player.
     * The playback duration is sent back from the Engine in the GetPlayerPositionMessageReply message.
     *
     * The default wait time for the reply is 1000 ms.
     * If the reply is not received within that time, a timeout occurs.
     *
     * @return The audio player's playback duration in milliseconds,
     *         or @c TIME_UNKNOWN if the current media duration is unknown or invalid.
     */
    int64_t getPlayerDuration();

    /**
     * Publishes GetPlayerPosition message to notify the Engine to send the current playback position of the audio player.
     * The playback position is sent back from the Engine in the GetPlayerPositionMessageReply message.
     *
     * The default wait time for the reply is 1000 ms.
     * If the reply is not received within that time, a timeout occurs.
     *
     * @return The audio player's playback duration in milliseconds,
     *         or @c TIME_UNKNOWN if the current media duration is unknown or invalid.
    */
    int64_t getPlayerPosition();

    /**
     * Provides the platform implementation of a change in audio playback state
     *
     * @param [in] state The new playback state
     */
    void playerActivityChanged(aasb::message::alexa::audioPlayer::PlayerActivity state);

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
    auto convertPlayerActivityToString(const aasb::message::alexa::audioPlayer::PlayerActivity& state) -> std::string;

    // to handle async reply
    using AudioPlayerPromise = std::promise<int64_t>;
    std::mutex m_promise_map_access_mutex;
    std::unordered_map<std::string, std::shared_ptr<AudioPlayerPromise>> m_promiseMap;

    int64_t waitForAsyncReply(const std::string& messageId);
    void addReplyMessagePromise(const std::string& messageId, std::shared_ptr<AudioPlayerPromise> promise);
    void removeReplyMessagePromise(const std::string& messageId);
    std::shared_ptr<AudioPlayerPromise> getReplyMessagePromise(const std::string& messageId);
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_AUDIOPLAYERHANDLER_H
