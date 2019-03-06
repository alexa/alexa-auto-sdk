/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AASB_ALEXA_MEDIAPLAYERHANDLER_H
#define AASB_ALEXA_MEDIAPLAYERHANDLER_H

#include <condition_variable>
#include <memory>
#include <string>

#include <AACE/Alexa/MediaPlayer.h>
#include "DirectiveDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

/**
 * Platform implementation for @c aace::alexa::MediaPlayer interface.
 *
 * The @c MediaPlayer implementation detail is following:
 * 1) It is assumed that a platform has its own media player.
 * 2) Platform media player doesn't implement @c MediaPlayer interface.
 * 3) All media player calls such as @c prepare, @c play etc. are converted into messages for the
 *    platform media player. These messages are delivered through @c DirectiveDispatcher instance.
 * 4) When platform media player has anything to notify the engine they encode the information in
 *    a message and send it back to the @c MediaPlayerHandler
 */
class MediaPlayerHandler : public aace::alexa::MediaPlayer {
public:
    static std::shared_ptr<MediaPlayerHandler> create(
        const std::string& name,
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher,
        std::string media_file);

    /// @name aace::alexa::MediaPlayer Functions
    /// @{
    bool prepare() override;
    bool prepare(const std::string& url) override;
    bool play() override;
    bool stop() override;
    bool pause() override;
    bool resume() override;
    int64_t getPosition() override;
    bool setPosition(int64_t position) override;
    /// @}

    void onMediaStateChangedEvent(const std::string& payload);
    void onMediaErrorEvent(const std::string& mediaErrorStr, const std::string& description);
    void onMediaPlayerPositionReceived(int64_t position);

private:
    MediaPlayerHandler() = default;

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // AudioPlayer or Alerts or SpeechSynthesizer
    std::string m_topic;

    std::weak_ptr<aasb::bridge::DirectiveDispatcher> m_directiveDispatcher;

    // Media File
    std::string m_media_file;

    std::condition_variable m_cv_position;
    std::mutex m_mutex;
    int64_t m_mediaplayer_position;
    bool m_mediaplayer_position_set;

    static MediaState toMediaState(const std::string& mediaState);
    static MediaError toMediaError(const std::string& mediaErrorStr);
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_MEDIAPLAYERHANDLER_H