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

#ifndef AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_ENGINE_SERVICE_H
#define AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_ENGINE_SERVICE_H

#include "AACE/Engine/Core/EngineService.h"
#include "AACE/TextToSpeech/TextToSpeech.h"

#include "TextToSpeechEngineImpl.h"
#include "TextToSpeechServiceInterface.h"
#include "TextToSpeechSynthesizerInterface.h"

namespace aace {
namespace engine {
namespace textToSpeech {

class TextToSpeechEngineService
        : public aace::engine::core::EngineService
        , public TextToSpeechServiceInterface
        , public std::enable_shared_from_this<TextToSpeechEngineService> {
public:
    DESCRIBE("aace.textToSpeech", VERSION("1.0"))

private:
    TextToSpeechEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~TextToSpeechEngineService() = default;

    // TextToSpeechServiceInterface
    std::shared_ptr<TextToSpeechSynthesizerInterface> getTextToSpeechProvider(const std::string& name) override;
    void registerTextToSpeechProvider(
        const std::string& textToSpeechProviderName,
        std::shared_ptr<TextToSpeechSynthesizerInterface> textToSpeechProvider) override;

protected:
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;
    bool initialize() override;
    bool shutdown() override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::textToSpeech::TextToSpeech> textToSpeech);

private:
    std::shared_ptr<TextToSpeechEngineImpl> m_textToSpeechEngineImpl;
    std::mutex m_textToSpeechProviderMutex;

    // Map to store Text To Speech provider name and the associated Text To Speech Providers
    std::unordered_map<std::string, std::shared_ptr<TextToSpeechSynthesizerInterface>>
        m_registeredTextToSpeechProviders;
};

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_ENGINE_SERVICE_H
