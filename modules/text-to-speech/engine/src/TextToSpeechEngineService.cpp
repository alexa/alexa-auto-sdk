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
#include "AACE/Engine/TextToSpeech/TextToSpeechEngineService.h"

namespace aace {
namespace engine {
namespace textToSpeech {

// String to identify log entries originating from this file.
static const std::string TAG("aace.textToSpeech.TextToSpeechEngineService");

// register the service
REGISTER_SERVICE(TextToSpeechEngineService);

TextToSpeechEngineService::TextToSpeechEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool TextToSpeechEngineService::initialize() {
    try {
        // register the TextToSpeechServiceInterface
        ThrowIfNot(
            registerServiceInterface<TextToSpeechServiceInterface>(shared_from_this()),
            "registerTextToSpeechServiceInterfaceFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool TextToSpeechEngineService::shutdown() {
    AACE_INFO(LX(TAG));
    if (m_textToSpeechEngineImpl != nullptr) {
        m_textToSpeechEngineImpl->shutdown();
        m_textToSpeechEngineImpl.reset();
    }
    m_registeredTextToSpeechProviders.clear();
    return true;
}

bool TextToSpeechEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::textToSpeech::TextToSpeech>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool TextToSpeechEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::textToSpeech::TextToSpeech> textToSpeech) {
    AACE_INFO(LX(TAG).m("Registering TextToSpeech platform interface"));
    try {
        ThrowIfNotNull(m_textToSpeechEngineImpl, "platformInterfaceAlreadyRegistered");

        m_textToSpeechEngineImpl =
            aace::engine::textToSpeech::TextToSpeechEngineImpl::create(textToSpeech, shared_from_this());
        ThrowIfNull(m_textToSpeechEngineImpl, "createTextToSpeechEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<TextToSpeechSynthesizerInterface> TextToSpeechEngineService::getTextToSpeechProvider(
    const std::string& name) {
    try {
        std::unique_lock<std::mutex> lock(m_textToSpeechProviderMutex);
        auto registeredTextToSpeechProviders = m_registeredTextToSpeechProviders;
        lock.unlock();
        auto it = registeredTextToSpeechProviders.find(name);
        ThrowIf(it == registeredTextToSpeechProviders.end(), "providerNotRegistered");
        return it->second;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("providerName", name));
        return nullptr;
    }
}

void TextToSpeechEngineService::registerTextToSpeechProvider(
    const std::string& textToSpeechProviderName,
    std::shared_ptr<TextToSpeechSynthesizerInterface> textToSpeechProvider) {
    try {
        ThrowIf(textToSpeechProviderName.empty(), "emptyTextToSpeechProviderName");
        ThrowIfNull(textToSpeechProvider, "nullTextToSpeechSynthesizerInterface");
        std::lock_guard<std::mutex> lock(m_textToSpeechProviderMutex);
        ThrowIf(
            m_registeredTextToSpeechProviders.find(textToSpeechProviderName) != m_registeredTextToSpeechProviders.end(),
            "textToSpeechProviderAlreadyRegistered");
        m_registeredTextToSpeechProviders[textToSpeechProviderName] = textToSpeechProvider;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aace
