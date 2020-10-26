/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Alexa/SpeechRecognizer.h>

namespace aace {
namespace alexa {

SpeechRecognizer::SpeechRecognizer() {
}

SpeechRecognizer::~SpeechRecognizer() = default;  // key function

bool SpeechRecognizer::holdToTalk() {
    return startCapture(Initiator::HOLD_TO_TALK);
}

bool SpeechRecognizer::tapToTalk() {
    return startCapture(Initiator::TAP_TO_TALK);
}

bool SpeechRecognizer::startCapture(
    Initiator initiator,
    uint64_t keywordBegin,
    uint64_t keywordEnd,
    const std::string& keyword) {
    if (auto m_speechRecognizerEngineInterface_lock = m_speechRecognizerEngineInterface.lock()) {
        return m_speechRecognizerEngineInterface_lock->onStartCapture(initiator, keywordBegin, keywordEnd, keyword);
    } else {
        return false;
    }
}

bool SpeechRecognizer::stopCapture() {
    if (auto m_speechRecognizerEngineInterface_lock = m_speechRecognizerEngineInterface.lock()) {
        return m_speechRecognizerEngineInterface_lock->onStopCapture();
    } else {
        return false;
    }
}

bool SpeechRecognizer::wakewordDetected(const std::string& wakeword) {
    return true;
}

void SpeechRecognizer::endOfSpeechDetected() {
}

void SpeechRecognizer::setEngineInterface(
    std::shared_ptr<aace::alexa::SpeechRecognizerEngineInterface> speechRecognizerEngineInterface) {
    m_speechRecognizerEngineInterface = speechRecognizerEngineInterface;
}

}  // namespace alexa
}  // namespace aace
