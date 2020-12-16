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

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/TextToSpeech/TextToSpeech.h"

namespace aace {
namespace textToSpeech {

// String to identify log entries originating from this file.
static const std::string TAG("aace.textToSpeech.TextToSpeech");

TextToSpeech::~TextToSpeech() = default;

bool TextToSpeech::prepareSpeech(
    const std::string& speechId,
    const std::string& text,
    const std::string& provider,
    const std::string& options) {
    try {
        auto m_ttsEngineInterface_lock = m_ttsEngineInterface.lock();
        ThrowIfNull(m_ttsEngineInterface_lock, "nullTTSEngineInterface");
        return m_ttsEngineInterface_lock->onPrepareSpeech(speechId, text, provider, options);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool TextToSpeech::getCapabilities(const std::string& requestId, const std::string& provider) {
    try {
        auto m_ttsEngineInterface_lock = m_ttsEngineInterface.lock();
        ThrowIfNull(m_ttsEngineInterface_lock, "nullTTSEngineInterface");
        return m_ttsEngineInterface_lock->onGetCapabilities(requestId, provider);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void TextToSpeech::setEngineInterface(std::shared_ptr<TextToSpeechEngineInterface> ttsEngineInterface) {
    m_ttsEngineInterface = ttsEngineInterface;
}

}  // namespace textToSpeech
}  // namespace aace
