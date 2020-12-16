/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_TEXTTOSPEECH_TEXTTOSPEECHHANDLER_H
#define SAMPLEAPP_TEXTTOSPEECH_TEXTTOSPEECHHANDLER_H

#include <AACE/Audio/AudioStream.h>
#include <AACE/TextToSpeech/TextToSpeech.h>
#include "SampleApp/Logger/LoggerHandler.h"
#include "SampleApp/Audio/AudioOutputProviderHandler.h"

namespace sampleApp {
namespace textToSpeech {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TextToSpeechHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class TextToSpeechHandler : public aace::textToSpeech::TextToSpeech /* isa PlatformInterface */ {
protected:
    TextToSpeechHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::weak_ptr<audio::AudioOutputProviderHandler> audioOutputProviderHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<TextToSpeechHandler> {
        return std::shared_ptr<TextToSpeechHandler>(new TextToSpeechHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::textToSpeech::TextToSpeech interface
    auto prepareSpeechCompleted(
        const std::string& speechId,
        std::shared_ptr<aace::audio::AudioStream> preparedAudio,
        const std::string& metadata) -> void override;
    auto prepareSpeechFailed(const std::string& speechId, const std::string& reason) -> void override;
    auto capabilitiesReceived(const std::string& requestId, const std::string& capabilities) -> void override;

private:
    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    /**
     * Setup all the event listeners.
     */
    auto setupUI() -> void;

    auto generateId() -> std::string;

private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};
    std::weak_ptr<audio::AudioOutputProviderHandler> m_audioOutputHandler{};
    /// Console
    std::weak_ptr<View> m_console{};
    std::atomic<long> m_idCounter;
};

}  // namespace textToSpeech
}  // namespace sampleApp

#endif  // SAMPLEAPP_TEXTTOSPEECH_TEXTTOSPEECHHANDLER_H
