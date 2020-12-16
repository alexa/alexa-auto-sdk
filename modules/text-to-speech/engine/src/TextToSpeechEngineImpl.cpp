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

#include <unordered_map>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/TextToSpeech/TextToSpeechEngineImpl.h"
#include "AACE/Engine/TextToSpeech/TextToSpeechSynthesizerInterface.h"
#include "AACE/Engine/Utils/String/StringUtils.h"
#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace textToSpeech {

// String to identify log entries originating from this file.
static const std::string TAG("aace.textToSpeech.TextToSpeechEngineImpl");

// Timeout for the prepare speech request after which the TextToSpeechEngineImpl will return with an error
static std::chrono::milliseconds DEFAULT_REQUEST_TIMEOUT(1000);

// Error string for request timeout
static const std::string REQUEST_TIMED_OUT = "REQUEST_TIMED_OUT";

// Error string for internal error
static const std::string INTERNAL_ERROR = "INTERNAL_ERROR";

// String to identify the request payload key in the options parameter passed for a speech synthesis request
static const std::string REQUEST_PAYLOAD_KEY = "requestPayload";

static const std::string EMPTY_STRING = "";

TextToSpeechEngineImpl::TextToSpeechEngineImpl(
    std::shared_ptr<aace::textToSpeech::TextToSpeech> textToSpeechPlatformInterface) :
        m_textToSpeechPlatformInterface(textToSpeechPlatformInterface) {
}

bool TextToSpeechEngineImpl::initialize(std::shared_ptr<TextToSpeechServiceInterface> textToSpeechServiceInterface) {
    m_textToSpeechServiceInterface = textToSpeechServiceInterface;
    return true;
}

std::shared_ptr<TextToSpeechEngineImpl> TextToSpeechEngineImpl::create(
    std::shared_ptr<aace::textToSpeech::TextToSpeech> textToSpeechPlatformInterface,
    std::shared_ptr<TextToSpeechServiceInterface> textToSpeechServiceInterface) {
    try {
        ThrowIfNull(textToSpeechPlatformInterface, "nullTextToSpeechPlatformInterface");
        ThrowIfNull(textToSpeechServiceInterface, "nullTextToSpeechServiceInterface");
        auto textToSpeechEngineImpl =
            std::shared_ptr<TextToSpeechEngineImpl>(new TextToSpeechEngineImpl(textToSpeechPlatformInterface));

        ThrowIfNot(
            textToSpeechEngineImpl->initialize(textToSpeechServiceInterface), "initializeTextToSpeechEngineImplFailed");

        // Set the Engine Interface reference
        textToSpeechPlatformInterface->setEngineInterface(textToSpeechEngineImpl);

        return textToSpeechEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool TextToSpeechEngineImpl::onPrepareSpeech(
    const std::string& speechId,
    const std::string& text,
    const std::string& provider,
    const std::string& options) {
    try {
        AACE_INFO(LX(TAG)
                      .sensitive("speechId", speechId)
                      .sensitive("text", text)
                      .d("provider", provider)
                      .d("options", options));
        ThrowIf(speechId.empty(), "emptySpeechId");
        ThrowIf(text.empty(), "emptyText");
        auto m_textToSpeechServiceInterface_lock = m_textToSpeechServiceInterface.lock();
        ThrowIfNull(m_textToSpeechServiceInterface_lock, "nullTextToSpeechServiceInterface");
        auto textToSpeechProvider = m_textToSpeechServiceInterface_lock->getTextToSpeechProvider(provider);
        ThrowIfNull(textToSpeechProvider, "nullTextToSpeechProvider");
        return executeOnPrepareSpeech(speechId, text, textToSpeechProvider, options);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}
bool TextToSpeechEngineImpl::onGetCapabilities(const std::string& requestId, const std::string& provider) {
    try {
        ThrowIf(requestId.empty(), "emptyRequestId");
        auto m_textToSpeechServiceInterface_lock = m_textToSpeechServiceInterface.lock();
        ThrowIfNull(m_textToSpeechServiceInterface_lock, "nullTextToSpeechServiceInterface");
        auto textToSpeechProvider = m_textToSpeechServiceInterface_lock->getTextToSpeechProvider(provider);
        ThrowIfNull(textToSpeechProvider, "nullTextToSpeechProvider");
        return executeOnGetCapabilities(requestId, textToSpeechProvider);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool TextToSpeechEngineImpl::executeOnPrepareSpeech(
    const std::string& speechId,
    const std::string& text,
    std::shared_ptr<TextToSpeechSynthesizerInterface> textToSpeechProvider,
    const std::string& options) {
    try {
        AACE_INFO(LX(TAG));
        ThrowIfNull(m_textToSpeechPlatformInterface, "nullTextToSpeechPlatformInterface");
        auto textToSpeechPlatformInterface = m_textToSpeechPlatformInterface;
        std::string requestPayload;
        std::string requestTimeout;
        if (!options.empty()) {
            nlohmann::json optionsPayload = nlohmann::json::parse(options);
            if (optionsPayload.contains(REQUEST_PAYLOAD_KEY)) {
                requestPayload = optionsPayload.at(REQUEST_PAYLOAD_KEY).dump();
            }
        }
        m_executor.submit(
            [speechId, text, textToSpeechProvider, requestTimeout, requestPayload, textToSpeechPlatformInterface] {
                try {
                    AACE_DEBUG(LX(TAG).m("Executing prepare speech"));
                    auto prepareSpeechFuture = textToSpeechProvider->prepareSpeech(speechId, text, requestPayload);
                    auto status = prepareSpeechFuture.wait_for(DEFAULT_REQUEST_TIMEOUT);
                    if (status == std::future_status::timeout) {
                        textToSpeechPlatformInterface->prepareSpeechFailed(speechId, REQUEST_TIMED_OUT);
                    } else {
                        auto prepareSpeechResult = prepareSpeechFuture.get();
                        auto speechId = prepareSpeechResult.getSpeechId();
                        auto failureReason = prepareSpeechResult.getFailureReason();
                        auto metadata = prepareSpeechResult.getSpeechMetadata();
                        auto synthesizedSpeech = prepareSpeechResult.getPreparedAudio();
                        if (!failureReason.empty()) {
                            textToSpeechPlatformInterface->prepareSpeechFailed(speechId, failureReason);
                        } else {
                            textToSpeechPlatformInterface->prepareSpeechCompleted(
                                speechId, synthesizedSpeech, metadata);
                        }
                    }
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                    textToSpeechPlatformInterface->prepareSpeechFailed(speechId, INTERNAL_ERROR);
                }
            });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool TextToSpeechEngineImpl::executeOnGetCapabilities(
    const std::string& requestId,
    std::shared_ptr<TextToSpeechSynthesizerInterface> textToSpeechProvider) {
    try {
        AACE_INFO(LX(TAG));
        ThrowIfNull(m_textToSpeechPlatformInterface, "nullTextToSpeechPlatformInterface");
        auto textToSpeechPlatformInterface = m_textToSpeechPlatformInterface;
        m_executor.submit([requestId, textToSpeechProvider, textToSpeechPlatformInterface] {
            AACE_DEBUG(LX(TAG).m("Getting capabilities"));
            auto capabilitiesFuture = textToSpeechProvider->getCapabilities(requestId);
            try {
                auto status = capabilitiesFuture.wait_for(DEFAULT_REQUEST_TIMEOUT);
                if (status == std::future_status::timeout) {
                    textToSpeechPlatformInterface->capabilitiesReceived(requestId, EMPTY_STRING);
                } else {
                    auto capabilities = capabilitiesFuture.get();
                    textToSpeechPlatformInterface->capabilitiesReceived(requestId, capabilities);
                }
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG).d("reason", ex.what()));
                textToSpeechPlatformInterface->capabilitiesReceived(requestId, EMPTY_STRING);
            }
        });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void TextToSpeechEngineImpl::shutdown() {
    if (m_textToSpeechPlatformInterface != nullptr) {
        m_textToSpeechPlatformInterface->setEngineInterface(nullptr);
        m_textToSpeechPlatformInterface.reset();
    }
    m_executor.shutdown();
}

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aace
