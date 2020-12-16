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

#include "AACE/Engine/TextToSpeech/PrepareSpeechResult.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace textToSpeech {

// String to identify log entries originating from this file.
static const std::string TAG("aace.textToSpeech.PrepareSpeechResult");

PrepareSpeechResult::PrepareSpeechResult(
    const std::string& speechId,
    std::shared_ptr<aace::audio::AudioStream> preparedAudio,
    const std::string& speechMetadata,
    const std::string& failureReason) :
        m_speechId(speechId),
        m_preparedAudio(preparedAudio),
        m_speechMetadata(speechMetadata),
        m_failureReason(failureReason) {
}

std::string PrepareSpeechResult::getSpeechId() const {
    return m_speechId;
}

std::string PrepareSpeechResult::getSpeechMetadata() const {
    return m_speechMetadata;
}

std::shared_ptr<aace::audio::AudioStream> PrepareSpeechResult::getPreparedAudio() {
    return m_preparedAudio;
}

std::string PrepareSpeechResult::getFailureReason() const {
    return m_failureReason;
}

void PrepareSpeechResult::setPreparedAudio(std::shared_ptr<aace::audio::AudioStream> preparedAudio) {
    m_preparedAudio = preparedAudio;
}

void PrepareSpeechResult::setSpeechId(const std::string& speechId) {
    m_speechId = speechId;
}

void PrepareSpeechResult::setFailureReason(const std::string& failureReason) {
    m_failureReason = failureReason;
}

void PrepareSpeechResult::setSpeechMetadata(const std::string& metadata) {
    m_speechMetadata = metadata;
}

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aace
