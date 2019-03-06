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
#ifndef AASB_ALEXA_PLATFORM_AUDIO_SPEECHRECOGNIZER_H
#define AASB_ALEXA_PLATFORM_AUDIO_SPEECHRECOGNIZER_H

#include <memory>

#include <AACE/Alexa/SpeechRecognizer.h>
#include <AACE/Audio/AudioCapture.h>

#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

/**
 * Implementation for @c aace::alexa::SpeechRecognizer.
 *
 * This implementation relies upon platform audio class @c aace::audio::AudioCapture to
 * provide microphone audio samples when request for @c startAudioInput arrives.
 */
class PlatformAudioSpeechRecognizer
        : public aace::alexa::SpeechRecognizer
        , public std::enable_shared_from_this<PlatformAudioSpeechRecognizer> {
public:
    /**
     * Creates a new instance of @c PlatformAudioSpeechRecognizer
     *
     * @param wakeworkDetectionEnabled Wakeword is enabled or not.
     * @param logger An instance of logger.
     * @param audioCapture For capturing audio samples from microphone.
     */
    static std::shared_ptr<PlatformAudioSpeechRecognizer> create(
        bool wakeworkDetectionEnabled,
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::shared_ptr<aace::audio::AudioCapture> audioCapture);

    /// @name aace::alexa::SpeechRecognizer Functions
    /// @{
    bool wakewordDetected(const std::string& wakeword) override;
    void endOfSpeechDetected() override;
    bool startAudioInput() override;
    bool stopAudioInput() override;
    /// @}

private:
    PlatformAudioSpeechRecognizer(
        bool wakeworkDetectionEnabled,
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::shared_ptr<aace::audio::AudioCapture> audioCapture);

    // Logger.
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // For obtaining audio samples from microphone.
    std::shared_ptr<aace::audio::AudioCapture> m_audioCapture;

    bool m_shouldStream;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_PLATFORM_AUDIO_SPEECHRECOGNIZER_H