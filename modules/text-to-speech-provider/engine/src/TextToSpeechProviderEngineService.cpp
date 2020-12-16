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
#include "AACE/Engine/TextToSpeech/TextToSpeechServiceInterface.h"
#include "AACE/Engine/TextToSpeechProvider/TextToSpeechProviderEngine.h"
#include "AACE/Engine/TextToSpeechProvider/TextToSpeechProviderEngineService.h"

#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace textToSpeechProvider {

using json = nlohmann::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.textToSpeechProvider.TextToSpeechProviderEngineService");

// String to uniquely identify this provider
const char* const PROVIDER_KEY = "text-to-speech-provider";

// register the service
REGISTER_SERVICE(TextToSpeechProviderEngineService);

TextToSpeechProviderEngineService::TextToSpeechProviderEngineService(
    const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool TextToSpeechProviderEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        json jconfiguration;
        jconfiguration = json::parse(*configuration);
        ThrowIfNull(jconfiguration, "emptyTextToSpeechProviderConfiguration");
        auto voices = jconfiguration.value("voices", json::array());
        ThrowIf(voices.empty(), "emptyVoiceArrayInTextToSpeechProviderConfiguration");
        std::set<std::string> voiceIds;
        for (auto& voice : voices) {
            ThrowIf(voice.value("voiceId", "").empty(), "missingVoiceId");
            ThrowIf(voice.value("locales", json::array()).empty(), "missingLocalesForVoiceId");
        }
        m_voiceConfiguration = jconfiguration.dump();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool TextToSpeechProviderEngineService::initialize() {
    try {
        // register the TextToSpeechProviderEngineService
        ThrowIfNot(
            registerServiceInterface<TextToSpeechProviderEngineService>(shared_from_this()),
            "registerTextToSpeechProviderEngineServiceFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool TextToSpeechProviderEngineService::shutdown() {
    AACE_INFO(LX(TAG));
    if (m_textToSpeechProviderEngine != nullptr) {
        m_textToSpeechProviderEngine->shutdown();
        m_textToSpeechProviderEngine.reset();
    }
    return true;
}

bool TextToSpeechProviderEngineService::postRegister() {
    try {
        auto alexaComponents =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "nullAlexaComponentInterface");

        auto messageSender = alexaComponents->getMessageSender();
        ThrowIfNull(messageSender, "nullMessageSender");

        auto exceptionSender = alexaComponents->getExceptionEncounteredSender();
        ThrowIfNull(exceptionSender, "exceptionSenderInvalid");

        auto defaultEndpointBuilder = alexaComponents->getDefaultEndpointBuilder();
        ThrowIfNull(defaultEndpointBuilder, "defaultEndpointBuilderInvalid");

        auto connectionManager = alexaComponents->getConnectionManager();
        ThrowIfNull(connectionManager, "connectionManagerInvalid");

        m_textToSpeechProviderEngine = aace::engine::textToSpeechProvider::TextToSpeechProviderEngine::create(
            defaultEndpointBuilder,
            exceptionSender,
            messageSender,
            connectionManager,
            m_voiceConfiguration,
            getContext());
        ThrowIfNull(m_textToSpeechProviderEngine, "nullTextToSpeechProviderEngine");

        auto textToSpeechServiceInterface =
            getContext()->getServiceInterface<aace::engine::textToSpeech::TextToSpeechServiceInterface>(
                "aace.textToSpeech");
        ThrowIfNull(textToSpeechServiceInterface, "nullTextToSpeechServiceInterface");
        textToSpeechServiceInterface->registerTextToSpeechProvider(PROVIDER_KEY, m_textToSpeechProviderEngine);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "postRegister").d("reason", ex.what()));
        return false;
    }
}

}  // namespace textToSpeechProvider
}  // namespace engine
}  // namespace aace
