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

#ifndef AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_ENGINE_IMPL_H
#define AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_ENGINE_IMPL_H

#include <unordered_map>
#include <unordered_set>

#include <AACE/Engine/Utils/Threading/Executor.h>

#include "AACE/TextToSpeech/TextToSpeech.h"
#include "AACE/TextToSpeech/TextToSpeechEngineInterface.h"
#include "TextToSpeechServiceInterface.h"

namespace aace {
namespace engine {
namespace textToSpeech {

class TextToSpeechEngineImpl : public aace::textToSpeech::TextToSpeechEngineInterface {
private:
    TextToSpeechEngineImpl(std::shared_ptr<aace::textToSpeech::TextToSpeech> textToSpeechPlatformInterface);

    bool initialize(std::shared_ptr<TextToSpeechServiceInterface> textToSpeechServiceInterface);

public:
    static std::shared_ptr<TextToSpeechEngineImpl> create(
        std::shared_ptr<aace::textToSpeech::TextToSpeech> textToSpeechPlatformInterface,
        std::shared_ptr<TextToSpeechServiceInterface> textToSpeechServiceInterface);

    // TextToSpeechEngineInterface
    bool onPrepareSpeech(
        const std::string& speechId,
        const std::string& text,
        const std::string& provider,
        const std::string& options) override;
    bool onGetCapabilities(const std::string& requestId, const std::string& provider) override;

    void shutdown();

private:
    bool executeOnPrepareSpeech(
        const std::string& speechId,
        const std::string& text,
        std::shared_ptr<TextToSpeechSynthesizerInterface> textToSpeechProvider,
        const std::string& options);
    bool executeOnGetCapabilities(
        const std::string& requestId,
        std::shared_ptr<TextToSpeechSynthesizerInterface> textToSpeechProvider);
    std::shared_ptr<aace::textToSpeech::TextToSpeech> m_textToSpeechPlatformInterface;
    std::weak_ptr<TextToSpeechServiceInterface> m_textToSpeechServiceInterface;

    // executor for speech synthesis requests
    aace::engine::utils::threading::Executor m_executor;
};

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_ENGINE_IMPL_H
