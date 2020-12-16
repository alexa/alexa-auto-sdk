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

#ifndef AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_ENGINE_SERVICE_H
#define AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_ENGINE_SERVICE_H

#include <unordered_map>

#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineService.h"
#include "AACE/Engine/TextToSpeech/TextToSpeechEngineService.h"

namespace aace {
namespace engine {
namespace textToSpeechProvider {

class TextToSpeechProviderEngineService
        : public aace::engine::core::EngineService
        , public std::enable_shared_from_this<TextToSpeechProviderEngineService> {
public:
    DESCRIBE(
        "aace.textToSpeechProvider",
        VERSION("1.0"),
        DEPENDS(aace::engine::textToSpeech::TextToSpeechEngineService),
        DEPENDS(aace::engine::propertyManager::PropertyManagerEngineService),
        DEPENDS(aace::engine::alexa::AlexaEngineService))

private:
    TextToSpeechProviderEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~TextToSpeechProviderEngineService() = default;

protected:
    bool initialize() override;
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool postRegister() override;
    bool shutdown() override;

private:
    std::shared_ptr<aace::engine::textToSpeechProvider::TextToSpeechProviderEngine> m_textToSpeechProviderEngine;
    std::string m_voiceConfiguration;
};

}  // namespace textToSpeechProvider
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_ENGINE_SERVICE_H
