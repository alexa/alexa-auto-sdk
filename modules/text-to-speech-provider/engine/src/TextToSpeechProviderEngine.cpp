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

#include <future>
#include <string>
#include <unordered_map>

#include "AACE/Alexa/AlexaProperties.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/TextToSpeechProvider/TextToSpeechProviderEngine.h"
#include "AACE/Engine/Utils/String/StringUtils.h"

#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace textToSpeechProvider {

using json = nlohmann::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.textToSpeechProvider.TextToSpeechProviderEngine");

// Key to identify voice name in the synthesize request
static const std::string VOICE_ID_KEY = "voiceId";

// Key to identify Assistant id
static const std::string ASSISTANT_ID_KEY = "assistantId";

// Key to identify voices name in the configuration
static const std::string VOICES_KEY = "voices";

// Key to identify locales in the configuration
static const std::string LOCALES_KEY = "locales";

// Key to identify locale in the synthesize request
static const std::string LOCALE_KEY = "locale";
static const std::string EMPTY_STRING = "";

// Failure reason to denote that the current locale for speech synthesis is not supported
static const std::string LOCALE_NOT_SUPPORTED = "LOCALE_NOT_SUPPORTED";

// Failure reason to denote any internal error
static const std::string INTERNAL_ERROR = "INTERNAL_ERROR";

// Failure reason to denote the requested voice for speech synthesis is not supported
static const std::string VOICE_NOT_SUPPORTED = "VOICE_NOT_SUPPORTED";

// Failure reason to denote the provider is not connected
static const std::string PROVIDER_NOT_CONNECTED = "PROVIDER_NOT_CONNECTED";

// Voice Id for Alexa
static const std::string ALEXA_VOICE_ID = "Alexa";

TextToSpeechProviderEngine::TextToSpeechProviderEngine() : alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG) {
}

bool TextToSpeechProviderEngine::initialize(
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager,
    const std::string& voiceConfiguration,
    std::shared_ptr<aace::engine::core::EngineContext> engineContext) {
    try {
        if (!voiceConfiguration.empty()) {
            createVoiceIdMapping(voiceConfiguration);
        }
        m_textToSpeechProviderCapabilityAgent =
            TextToSpeechProviderCapabilityAgent::create(shared_from_this(), exceptionSender, messageSender);
        ThrowIfNull(m_textToSpeechProviderCapabilityAgent, "nullTextToSpeechProviderCapabilityAgent");

        connectionManager->addConnectionStatusObserver(shared_from_this());
        defaultEndpointBuilder->withCapability(
            m_textToSpeechProviderCapabilityAgent, m_textToSpeechProviderCapabilityAgent);

        m_propertyManager =
            engineContext->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        auto m_propertyManager_lock = m_propertyManager.lock();
        ThrowIfNull(m_propertyManager_lock, "nullPropertyManagerServiceInterface");

        // Add listener to LOCALE property
        ThrowIfNot(
            m_propertyManager_lock->addListener(aace::alexa::property::LOCALE, shared_from_this()),
            "failedToAddListener");

        m_currentLocale = m_propertyManager_lock->getProperty(aace::alexa::property::LOCALE);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<TextToSpeechProviderEngine> TextToSpeechProviderEngine::create(
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager,
    const std::string& voiceConfiguration,
    std::shared_ptr<aace::engine::core::EngineContext> engineContext) {
    try {
        auto textToSpeechProviderEngine = std::shared_ptr<TextToSpeechProviderEngine>(new TextToSpeechProviderEngine());

        ThrowIfNot(
            textToSpeechProviderEngine->initialize(
                defaultEndpointBuilder,
                exceptionSender,
                messageSender,
                connectionManager,
                voiceConfiguration,
                engineContext),
            "initializeTextToSpeechProviderEngineFailed");

        return textToSpeechProviderEngine;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

std::future<aace::engine::textToSpeech::PrepareSpeechResult> TextToSpeechProviderEngine::prepareSpeech(
    const std::string& speechId,
    const std::string& text,
    const std::string& requestPayload) {
    AACE_INFO(LX(TAG).sensitive("speechId", speechId).sensitive("text", text).d("requestPayload", requestPayload));
    auto prepareSpeechPromise = std::make_shared<std::promise<aace::engine::textToSpeech::PrepareSpeechResult>>();
    std::future<aace::engine::textToSpeech::PrepareSpeechResult> prepareSpeechFuture;
    try {
        prepareSpeechFuture = prepareSpeechPromise->get_future();
        std::lock_guard<std::mutex> lock(m_mutex);
        ThrowIf(m_speechRequestsMap.find(speechId) != m_speechRequestsMap.end(), "requestWithDuplicateSpeechId");

        auto connectionStatus = getCurrentConnectionStatus();
        if (connectionStatus !=
            alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED) {
            AACE_ERROR(LX(TAG).m("Provider is not connected").d("connection status", connectionStatus));
            prepareSpeechPromise->set_value(createPrepareSpeechFailedResponse(speechId, PROVIDER_NOT_CONNECTED));
            return prepareSpeechFuture;
        }

        if (requestPayload.empty()) {
            m_speechRequestsMap[speechId] = prepareSpeechPromise;
            m_textToSpeechProviderCapabilityAgent->prepareSpeech(speechId, text, EMPTY_STRING);
            return prepareSpeechFuture;
        }

        json payload = json::parse(requestPayload);
        auto locale = payload.value(LOCALE_KEY, EMPTY_STRING);
        auto voiceId = payload.value(VOICE_ID_KEY, EMPTY_STRING);

        if (!validateLocale(voiceId, locale)) {
            prepareSpeechPromise->set_value(createPrepareSpeechFailedResponse(speechId, LOCALE_NOT_SUPPORTED));
            return prepareSpeechFuture;
        }
        std::string assistantId;
        if (voiceId != ALEXA_VOICE_ID) {
            auto it = m_voiceIdToAssistantIdMap.find(voiceId);
            if (it == m_voiceIdToAssistantIdMap.end()) {
                AACE_ERROR(LX(TAG).m("Requested voice ID for preparing speech is not supported").d("voiceId", voiceId));
                prepareSpeechPromise->set_value(createPrepareSpeechFailedResponse(speechId, VOICE_NOT_SUPPORTED));
                return prepareSpeechFuture;
            }
            assistantId = it->second;
        }
        m_speechRequestsMap[speechId] = prepareSpeechPromise;
        m_textToSpeechProviderCapabilityAgent->prepareSpeech(speechId, text, assistantId);
        return prepareSpeechFuture;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("speechId", speechId));
        prepareSpeechPromise->set_value(createPrepareSpeechFailedResponse(speechId, INTERNAL_ERROR));
        return prepareSpeechFuture;
    }
}

std::future<std::string> TextToSpeechProviderEngine::getCapabilities(const std::string& requestId) {
    AACE_INFO(LX(TAG).d("requestId", requestId));
    std::lock_guard<std::mutex> lock(m_mutex);
    auto getCapabilitiesPromise = std::make_shared<std::promise<std::string>>();
    std::future<std::string> getCapabilitiesPromiseFuture;
    try {
        getCapabilitiesPromiseFuture = getCapabilitiesPromise->get_future();
        if (m_currentLocale.empty()) {
            auto m_propertyManager_lock = m_propertyManager.lock();
            ThrowIfNull(m_propertyManager_lock, "nullPropertyManagerServiceInterface");
            m_currentLocale = m_propertyManager_lock->getProperty(aace::alexa::property::LOCALE);
        }
        // Construct the capability
        json voices = {{"voices", json::array()}};
        for (auto& voiceId : m_voiceIdToAssistantIdMap) {
            // Current locale is not in the supported locales list
            auto supportedLocales = m_voiceIdToLocalesMap.find(voiceId.first)->second;
            if (std::find(supportedLocales.begin(), supportedLocales.end(), m_currentLocale) ==
                supportedLocales.end()) {
                AACE_ERROR(LX(TAG).m("Current locale is not supported").d("currentLocale", m_currentLocale));
                getCapabilitiesPromise->set_value(EMPTY_STRING);
                return getCapabilitiesPromiseFuture;
            }

            // clang-format off
            json voice = {
                {VOICE_ID_KEY, voiceId.first},
                {LOCALES_KEY , {m_currentLocale}}
            };
            // clang-format on

            voices.at("voices").push_back(voice);
        }

        // clang-format off
        json defaultAlexaVoice = {
            {VOICE_ID_KEY, ALEXA_VOICE_ID},
            {LOCALES_KEY , {m_currentLocale}}
        };
        // clang-format on

        voices.at("voices").push_back(defaultAlexaVoice);

        // clang-format off
        json capabilities = {
            {PROVIDER_KEY, voices}
        };
        // clang-format on
        getCapabilitiesPromise->set_value(capabilities.dump());
        return getCapabilitiesPromiseFuture;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        getCapabilitiesPromise->set_exception(std::current_exception());
        return getCapabilitiesPromiseFuture;
    }
}

void TextToSpeechProviderEngine::prepareSpeechCompleted(
    const std::string& speechId,
    std::shared_ptr<aace::audio::AudioStream> preparedAudio,
    const std::string& metadata) {
    AACE_INFO(LX(TAG).d("speechId", speechId).d("metadata", metadata));
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_speechRequestsMap.find(speechId);
    if (it == m_speechRequestsMap.end()) {
        AACE_WARN(LX(TAG)
                      .m("No active request with matching speechId. Ignoring prepare speech response")
                      .d("speech ID", speechId));
        return;
    }
    auto prepareSpeechPromise = it->second;
    prepareSpeechPromise->set_value(
        aace::engine::textToSpeech::PrepareSpeechResult(speechId, preparedAudio, EMPTY_STRING, EMPTY_STRING));
    m_speechRequestsMap.erase(speechId);
}

void TextToSpeechProviderEngine::prepareSpeechFailed(const std::string& speechId, const std::string& reason) {
    AACE_INFO(LX(TAG).d("speechId", speechId).d("reason", reason));
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_speechRequestsMap.find(speechId);
    if (it == m_speechRequestsMap.end()) {
        AACE_WARN(LX(TAG)
                      .m("No active request with matching speechId. Ignoring prepare speech response")
                      .d("speech ID", speechId));
        return;
    }
    auto prepareSpeechPromise = it->second;
    prepareSpeechPromise->set_value(
        aace::engine::textToSpeech::PrepareSpeechResult(speechId, nullptr, EMPTY_STRING, reason));
    m_speechRequestsMap.erase(speechId);
}

void TextToSpeechProviderEngine::propertyChanged(const std::string& key, const std::string& newValue) {
    AACE_INFO(LX(TAG).d("key", key).sensitive("newValue", newValue));
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentLocale = newValue;
}

void TextToSpeechProviderEngine::onConnectionStatusChanged(
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason) {
    std::lock_guard<std::mutex> lock(m_connectionMutex);
    m_connectionStatus = status;
}

void TextToSpeechProviderEngine::createVoiceIdMapping(const std::string& voiceConfiguration) {
    json voiceConfig = json::parse(voiceConfiguration);
    for (auto& voice : voiceConfig.at(VOICES_KEY)) {
        auto assistantId = voice.value(ASSISTANT_ID_KEY, EMPTY_STRING);
        auto locales = voice.value(LOCALES_KEY, json::array());
        auto voiceId = voice.at(VOICE_ID_KEY);
        m_voiceIdToAssistantIdMap[voiceId] = assistantId;
        std::vector<std::string> supportedLocales;
        for (auto& locale : locales) {
            supportedLocales.push_back(locale);
        }
        m_voiceIdToLocalesMap[voiceId] = supportedLocales;
    }
}

aace::engine::textToSpeech::PrepareSpeechResult TextToSpeechProviderEngine::createPrepareSpeechFailedResponse(
    const std::string& speechId,
    const std::string& reason) {
    return aace::engine::textToSpeech::PrepareSpeechResult(speechId, nullptr, EMPTY_STRING, reason);
}

bool TextToSpeechProviderEngine::validateLocale(const std::string& voiceId, const std::string& locale) {
    try {
        AACE_DEBUG(LX(TAG).d("voiceId", voiceId).d("locale", locale));
        if (locale.empty()) {
            AACE_ERROR(LX(TAG)
                           .m("Requested locale for preparing speech is empty")
                           .d("locale", locale)
                           .d("currentLocale", m_currentLocale));
            return false;
        }

        if (m_currentLocale.empty()) {
            auto m_propertyManager_lock = m_propertyManager.lock();
            ThrowIfNull(m_propertyManager_lock, "nullPropertyManagerServiceInterface");
            m_currentLocale = m_propertyManager_lock->getProperty(aace::alexa::property::LOCALE);
        }

        // The requested locale for speech synthesis does not match the current locale
        if (!aace::engine::utils::string::equal(locale, m_currentLocale)) {
            AACE_ERROR(LX(TAG)
                           .m("Requested locale for preparing speech is not the current locale")
                           .d("locale", locale)
                           .d("currentLocale", m_currentLocale));
            return false;
        }

        // Current locale is not in the supported locales list
        auto iterator = m_voiceIdToLocalesMap.find(voiceId);
        if (iterator != m_voiceIdToLocalesMap.end()) {
            auto supportedLocales = iterator->second;
            if (!supportedLocales.empty() &&
                std::find(supportedLocales.begin(), supportedLocales.end(), m_currentLocale) ==
                    supportedLocales.end()) {
                AACE_ERROR(LX(TAG).m("Current locale is not supported").d("currentLocale", m_currentLocale));
                return false;
            }
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status TextToSpeechProviderEngine::
    getCurrentConnectionStatus() {
    std::lock_guard<std::mutex> lock(m_connectionMutex);
    return m_connectionStatus;
}
void TextToSpeechProviderEngine::doShutdown() {
    if (m_textToSpeechProviderCapabilityAgent != nullptr) {
        m_textToSpeechProviderCapabilityAgent->shutdown();
        m_textToSpeechProviderCapabilityAgent.reset();
    }
    for (auto speechId : m_speechRequestsMap) {
        speechId.second.reset();
    }
    m_speechRequestsMap.clear();
    m_voiceIdToAssistantIdMap.clear();
    m_voiceIdToLocalesMap.clear();
}

}  // namespace textToSpeechProvider
}  // namespace engine
}  // namespace aace
