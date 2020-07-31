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

#ifndef SAMPLEAPP_AUDIO_AUDIOINPUTPROVIDERHANDLER_H
#define SAMPLEAPP_AUDIO_AUDIOINPUTPROVIDERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Audio/AudioInputProvider.h>
#include <AACE/Audio/AudioStream.h>

#include <queue>
#include <fstream>

namespace sampleApp {
namespace audio {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioInputProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class DefaultAudioInput;

class AudioInputProviderHandler : public aace::audio::AudioInputProvider /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    AudioInputProviderHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        bool setup = true);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AudioInputProviderHandler> {
        return std::shared_ptr<AudioInputProviderHandler>(new AudioInputProviderHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;
    auto setupUI() -> void;

    // aace::audio::AudioInputProvider interface
    std::shared_ptr<aace::audio::AudioInput> openChannel(const std::string& name, AudioInputType type) override;

private:
    std::weak_ptr<View> m_console{};
    std::weak_ptr<View> m_alertStateView{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;

    std::shared_ptr<DefaultAudioInput> m_sharedAudioInput;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DefaultAudioInput
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class DefaultAudioInput : public aace::audio::AudioInput {
private:
    DefaultAudioInput() = default;

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<DefaultAudioInput> {
        return std::shared_ptr<DefaultAudioInput>(new DefaultAudioInput(args...));
    }

    bool setStream(std::shared_ptr<aace::audio::AudioStream> stream);

    // aace::audio::AudioInput
    bool startAudioInput() override;
    bool stopAudioInput() override;

private:
    Executor m_executer;
    bool m_running;

    std::shared_ptr<aace::audio::AudioStream> m_stream;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FileAudioStream
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class FileAudioStream : public aace::audio::AudioStream {
private:
    FileAudioStream() = default;

public:
    static std::shared_ptr<FileAudioStream> create(const std::string& path);

    bool open(const std::string& path);

    // aace::audio::AudioStream
    ssize_t read(char* data, const size_t size) override;
    bool isClosed() override;

private:
    std::ifstream m_stream;
    bool m_closed;
};

}  // namespace audio
}  // namespace sampleApp

#endif  // SAMPLEAPP_AUDIO_AUDIOINPUTPROVIDERHANDLER_H
