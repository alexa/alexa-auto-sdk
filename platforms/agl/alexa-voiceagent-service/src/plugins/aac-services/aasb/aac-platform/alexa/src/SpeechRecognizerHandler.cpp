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
#include "SpeechRecognizerHandler.h"
#include "PlatformSpecificLoggingMacros.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace alexa {

const std::string TAG = "aasb::alexa::SpeechRecognizerHandler";
const std::string TOPIC = "SpeechRecognizer";

std::shared_ptr<SpeechRecognizerHandler> SpeechRecognizerHandler::create(
    bool wakeworkDetectionEnabled,
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    auto speechRecognizerHandler =
        std::shared_ptr<SpeechRecognizerHandler>(new SpeechRecognizerHandler(wakeworkDetectionEnabled));

    speechRecognizerHandler->m_logger = logger;
    speechRecognizerHandler->m_responseDispatcher = responseDispatcher;

    return speechRecognizerHandler;
}

SpeechRecognizerHandler::SpeechRecognizerHandler(bool wakeworkDetectionEnabled) :
        aace::alexa::SpeechRecognizer(wakeworkDetectionEnabled),
        m_shouldStopStreamingFile(false) {
}

bool SpeechRecognizerHandler::wakewordDetected(const std::string& wakeword) {
    m_logger->log(Level::INFO, TAG, __FUNCTION__);

    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        responseDispatcher->sendDirective(TOPIC, __FUNCTION__, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
    }
    return true;
}

void SpeechRecognizerHandler::endOfSpeechDetected() {
    m_logger->log(Level::INFO, TAG, __FUNCTION__);
    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        responseDispatcher->sendDirective(TOPIC, __FUNCTION__, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
    }
}

}  // namespace alexa
}  // namespace aasb