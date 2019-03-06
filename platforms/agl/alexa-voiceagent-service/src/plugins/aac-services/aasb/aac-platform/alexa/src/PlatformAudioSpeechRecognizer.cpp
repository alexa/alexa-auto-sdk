/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <sstream>

#include "PlatformAudioSpeechRecognizer.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace alexa {

static const std::string TAG = "aasb::alexa::PlatformAudioSpeechRecognizer";

std::shared_ptr<PlatformAudioSpeechRecognizer> PlatformAudioSpeechRecognizer::create(
    bool wakeworkDetectionEnabled,
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::shared_ptr<aace::audio::AudioCapture> audioCapture) {
    return std::shared_ptr<PlatformAudioSpeechRecognizer>(
        new PlatformAudioSpeechRecognizer(wakeworkDetectionEnabled, logger, audioCapture));
}

PlatformAudioSpeechRecognizer::PlatformAudioSpeechRecognizer(
    bool wakeworkDetectionEnabled,
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::shared_ptr<aace::audio::AudioCapture> audioCapture) :
        aace::alexa::SpeechRecognizer(wakeworkDetectionEnabled),
        m_logger(logger),
        m_audioCapture(audioCapture) {
}

bool PlatformAudioSpeechRecognizer::wakewordDetected(const std::string& wakeword) {
    m_logger->log(Level::VERBOSE, TAG, "Wakeword detected");
    return true;
}

void PlatformAudioSpeechRecognizer::endOfSpeechDetected() {
    m_logger->log(Level::VERBOSE, TAG, "End of speech detected");
}

bool PlatformAudioSpeechRecognizer::startAudioInput() {
    m_logger->log(Level::INFO, TAG, "Start capturing audio input");

    std::weak_ptr<PlatformAudioSpeechRecognizer> thisWk(shared_from_this());
    bool result = m_audioCapture->startAudioInput([thisWk](const int16_t* data, const size_t size) {
        auto thisSh = thisWk.lock();
        if (thisSh) {
            return thisSh->write(data, size);
        }
        return static_cast<ssize_t>(-1);
    });

    if (!result) {
        m_logger->log(Level::WARN, TAG, "Start capturing audio input failed");
    }

    return result;
}

bool PlatformAudioSpeechRecognizer::stopAudioInput() {
    m_logger->log(Level::INFO, TAG, "Stop capturing audio input");

    bool result = m_audioCapture->stopAudioInput();

    if (!result) {
        m_logger->log(Level::WARN, TAG, "Stop capturing audio input failed");
    }

    return result;
}

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_SPEECHRECOGNIZERHANDLER_H