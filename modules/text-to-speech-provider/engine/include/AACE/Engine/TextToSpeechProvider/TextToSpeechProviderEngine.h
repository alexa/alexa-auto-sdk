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

#ifndef AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_ENGINE_H
#define AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_ENGINE_H

#include <unordered_map>

#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <Endpoints/EndpointBuilder.h>

#include "AACE/Engine/PropertyManager/PropertyListenerInterface.h"
#include "AACE/Engine/PropertyManager/PropertyManagerEngineService.h"
#include "AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h"
#include "AACE/Engine/TextToSpeech/PrepareSpeechResult.h"
#include "AACE/Engine/TextToSpeech/TextToSpeechSynthesizerInterface.h"
#include "TextToSpeechProviderCapabilityAgent.h"

namespace aace {
namespace engine {
namespace textToSpeechProvider {

extern const char* const PROVIDER_KEY;

class TextToSpeechProviderEngine
        : public aace::engine::textToSpeech::TextToSpeechSynthesizerInterface
        , public aace::engine::textToSpeechProvider::TextToSpeechProviderInterface
        , public aace::engine::propertyManager::PropertyListenerInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<TextToSpeechProviderEngine> {
private:
    TextToSpeechProviderEngine();

    bool initialize(
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager,
        const std::string& voiceConfiguration,
        std::shared_ptr<aace::engine::core::EngineContext> engineContext);

public:
    static std::shared_ptr<TextToSpeechProviderEngine> create(
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager,
        const std::string& voiceConfiguration,
        std::shared_ptr<aace::engine::core::EngineContext> engineContext);

    // TextToSpeechSynthesizerInterface
    std::future<aace::engine::textToSpeech::PrepareSpeechResult> prepareSpeech(
        const std::string& speechId,
        const std::string& text,
        const std::string& requestPayload) override;
    std::future<std::string> getCapabilities(const std::string& requestId) override;

    // TextToSpeechProviderInterface
    // Function to denote successful synthesis of speech. Called by the Capability Agent
    void prepareSpeechCompleted(
        const std::string& speechId,
        std::shared_ptr<aace::audio::AudioStream> preparedAudio,
        const std::string& metadata) override;
    void prepareSpeechFailed(const std::string& speechId, const std::string& reason) override;

    // PropertyListenerInterface
    void propertyChanged(const std::string& key, const std::string& newValue) override;

    // ConnectionStatusObserverInterface
    void onConnectionStatusChanged(
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason)
        override;

    void doShutdown() override;

private:
    void createVoiceIdMapping(const std::string& voiceConfiguration);
    aace::engine::textToSpeech::PrepareSpeechResult createPrepareSpeechFailedResponse(
        const std::string& speechId,
        const std::string& reason);
    bool validateLocale(const std::string& voiceId, const std::string& locale);
    alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status getCurrentConnectionStatus();

private:
    std::shared_ptr<aace::engine::textToSpeechProvider::TextToSpeechProviderCapabilityAgent>
        m_textToSpeechProviderCapabilityAgent;
    std::weak_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> m_propertyManager;
    std::unordered_map<std::string, std::shared_ptr<std::promise<aace::engine::textToSpeech::PrepareSpeechResult>>>
        m_speechRequestsMap;
    std::unordered_map<std::string, std::string> m_voiceIdToAssistantIdMap;
    std::unordered_map<std::string, std::vector<std::string>> m_voiceIdToLocalesMap;
    std::string m_currentLocale;
    std::mutex m_mutex;
    std::mutex m_connectionMutex;
    alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status m_connectionStatus =
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED;
};

}  // namespace textToSpeechProvider
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_ENGINE_H
