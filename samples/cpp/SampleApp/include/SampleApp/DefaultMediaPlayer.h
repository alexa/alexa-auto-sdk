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

#ifndef SAMPLEAPP_DEFAULTMEDIAPLAYER_H
#define SAMPLEAPP_DEFAULTMEDIAPLAYER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Alexa/MediaPlayer.h>
#include <AACE/Alexa/Speaker.h>

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DefaultMediaPlayer
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class DefaultMediaPlayer : public aace::alexa::MediaPlayer, public aace::alexa::Speaker {
  private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

    std::string m_identity{};
    std::string m_tempPath{};

  protected:
    DefaultMediaPlayer(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler, const std::string &identity = "Untitled");

  public:
    template <typename... Args> static auto create(Args &&... args) -> std::shared_ptr<DefaultMediaPlayer> {
        return std::shared_ptr<DefaultMediaPlayer>(new DefaultMediaPlayer(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::alexa::MediaPlayer interface

    auto prepare() -> bool override;
    auto prepare(const std::string &url) -> bool override;
    auto play() -> bool override;
    auto stop() -> bool override;
    auto pause() -> bool override;
    auto resume() -> bool override;
    auto getPosition() -> int64_t override;
    auto setPosition(int64_t position) -> bool override;

    // aace::alexa::Speaker interface

    auto setVolume(int8_t volume) -> bool override;
    auto adjustVolume(int8_t delta) -> bool override;
    auto setMute(bool mute) -> bool override;
    auto getVolume() -> int8_t override;
    auto isMuted() -> bool override;

  private:
    std::shared_ptr<ApplicationContext> m_applicationContext{};
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string &message) -> void;
    auto setupUI() -> void;
};

} // namespace sampleApp

#endif // SAMPLEAPP_DEFAULTMEDIAPLAYER_H
