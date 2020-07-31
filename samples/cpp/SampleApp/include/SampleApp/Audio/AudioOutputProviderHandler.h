/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_AUDIO_AUDIOOUTPUTPROVIDERHANDLER_H
#define SAMPLEAPP_AUDIO_AUDIOOUTPUTPROVIDERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/ApplicationContext.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Audio/AudioOutputProvider.h>
#include <AACE/Audio/AudioStream.h>

#include <memory>
#include <mutex>
#include <chrono>

namespace sampleApp {
namespace audio {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioOutputProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class DefaultAudioOutput;

class AudioOutputProviderHandler : public aace::audio::AudioOutputProvider /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    AudioOutputProviderHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        bool setup = true);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AudioOutputProviderHandler> {
        return std::shared_ptr<AudioOutputProviderHandler>(new AudioOutputProviderHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;
    auto setupUI() -> void;

    // aace::audio::AudioOutputProvider interface
    std::shared_ptr<aace::audio::AudioOutput> openChannel(const std::string& name, AudioOutputType type) override;

private:
    std::weak_ptr<View> m_console{};
    std::weak_ptr<View> m_alertStateView{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DefaultAudioOutput
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class DefaultAudioOutput
        : public aace::audio::AudioOutput
        , public std::enable_shared_from_this<DefaultAudioOutput> {
private:
    DefaultAudioOutput(
        std::weak_ptr<ApplicationContext> applicationContext,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        const std::string& name,
        std::chrono::milliseconds minPlayDuration = std::chrono::milliseconds(1),
        std::chrono::milliseconds maxPlayDuration = std::chrono::milliseconds(1),
        bool enableOutput = false);

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto writeStreamToFile(std::shared_ptr<aace::audio::AudioStream> stream) -> void;

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::unique_ptr<DefaultAudioOutput> {
        return std::unique_ptr<DefaultAudioOutput>(new DefaultAudioOutput(args...));
    }

    // aace::audio::AudioOutput
    bool prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) override;
    bool prepare(const std::string& url, bool repeating) override;
    bool play() override;
    bool stop() override;
    bool pause() override;
    bool resume() override;
    int64_t getPosition() override;
    bool setPosition(int64_t position) override;
    int64_t getDuration() override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(MutedState state) override;

private:
    std::weak_ptr<ApplicationContext> m_applicationContext{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

    Executor m_executer;

    std::string m_name;
    std::chrono::milliseconds m_minPlayDuration;
    std::chrono::milliseconds m_maxPlayDuration;
    bool m_enableOutput;

    int64_t m_position;

    bool m_playing;
    bool m_paused;
    std::condition_variable m_cv;
    std::mutex m_mutex;
};

}  // namespace audio
}  // namespace sampleApp

#endif  // SAMPLEAPP_AUDIO_AUDIOOUTPUTPROVIDERHANDLER_H
