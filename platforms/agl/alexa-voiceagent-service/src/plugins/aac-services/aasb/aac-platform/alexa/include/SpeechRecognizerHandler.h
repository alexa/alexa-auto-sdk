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
#ifndef AASB_ALEXA_SPEECHRECOGNIZERHANDLER_H
#define AASB_ALEXA_SPEECHRECOGNIZERHANDLER_H

#include <atomic>
#include <memory>

#include <AACE/Alexa/SpeechRecognizer.h>
#include "DirectiveDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

/**
 * Implementation for @c aace::alexa::SpeechRecognizer.
 *
 * The @c SpeechRecognizer implementation detail is following:
 * 1) It is assumed that a platform has its own way to obtain audio samples from microphone.
 * 2) Platform microphone input channel doesn't implement @c SpeechRecognizer interface.
 * 3) All Speech Recognizer calls such as @c startAudioInput, @c stopAudioInput etc. are
 *    converted into messages for the platform microphone input channel. These messages are
 *    delivered through @c DirectiveDispatcher instance.
 * 4) When platform microphone input channel has anything to notify the engine they encode the
 *    information in a message and send it back to the @c SpeechRecognizerHandler.
 */
class SpeechRecognizerHandler : public aace::alexa::SpeechRecognizer {
public:
    static std::shared_ptr<SpeechRecognizerHandler> create(
        bool wakeworkDetectionEnabled,
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    /// @name aace::alexa::SpeechRecognizer Functions
    /// @{
    bool wakewordDetected(const std::string& wakeword) override;
    void endOfSpeechDetected() override;
    bool startAudioInput() override;
    bool stopAudioInput() override;
    /// @}

private:
    SpeechRecognizerHandler(bool wakeworkDetectionEnabled);

    // Logger.
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // State to store if audio sample streaming to the file should continue or stop.
    std::atomic<bool> m_shouldStopStreamingFile;

    // DirectiveDispatcher to send status info.
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> m_directiveDispatcher;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_SPEECHRECOGNIZERHANDLER_H