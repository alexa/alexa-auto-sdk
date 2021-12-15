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

#ifndef SAMPLEAPP_AUDIO_AUDIOINPUTPROVIDERHANDLER_H
#define SAMPLEAPP_AUDIO_AUDIOINPUTPROVIDERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <queue>
#include <fstream>

namespace sampleApp {
namespace audio {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioInputProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AudioInputProviderHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    AudioInputProviderHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker,
        bool setup = true);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AudioInputProviderHandler> {
        return std::shared_ptr<AudioInputProviderHandler>(new AudioInputProviderHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;
    auto setupUI() -> void;

private:
    auto subscribeToAASBMessages() -> void;

    /**
     * Handles the StartAudioInputMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleStartAudioInputMessage(const std::string& message);

    /**
     * Handles the StopAudioInputMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleStopAudioInputMessage(const std::string& message);

    /**
     * Provides the implementation for starting audio input.
     */
    void startAudioInput(std::shared_ptr<aace::core::MessageStream> stream);

    /**
     * Provides the implementation for stopping audio input.
     */
    void stopAudioInput();

private:
    bool m_running;
    Executor m_executer;
    std::string m_path;
    std::ifstream m_stream;
    std::weak_ptr<View> m_console{};
    std::weak_ptr<View> m_alertStateView{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    ssize_t read(char* data, const size_t size);
};

}  // namespace audio
}  // namespace sampleApp

#endif  // SAMPLEAPP_AUDIO_AUDIOINPUTPROVIDERHANDLER_H
