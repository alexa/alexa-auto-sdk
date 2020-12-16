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

#include <string>
#include <thread>

#include <AVSCommon/Utils/SDS/ReaderPolicy.h>

#include "AACE/Engine/Alexa/AudioChannelEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/TextToSpeechProvider/TextToSpeechProviderCapabilityAgent.h"

namespace aace {
namespace engine {
namespace textToSpeechProvider {

// String to identify log entries originating from this file.
static const std::string TAG("aace.textToSpeechProvider.TextToSpeechProviderCapabilityAgent");

/// The namespace for this capability agent.
static const std::string NAMESPACE{"SpeechSynthesizer.Generator"};

/// The HandleSpeech directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName HANDLE_SPEECH{NAMESPACE, "HandleSpeech"};

/// Prepare Speech capability constants
/// Prepare Speech interface type
static const std::string PREPARE_SPEECH_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
/// Prepare Speech interface name
static const std::string PREPARE_SPEECH_CAPABILITY_INTERFACE_NAME = "SpeechSynthesizer.Generator";
/// Prepare Speech interface version
static const std::string PREPARE_SPEECH_CAPABILITY_INTERFACE_VERSION = "1.0";

/// String to identify text input
static const std::string TEXT_TYPE = "TEXT";
/// String to identify SSML input
static const std::string SSML_TYPE = "SSML";

/// Name of the event for to request speech synthesis
static const std::string SYNTHESIZE_EVENT = "Synthesize";

/// Prefix for content ID prefix in the url property of the directive payload.
static const std::string CID_PREFIX = "cid:";

/// Key for url property of the directive payload.
static const std::string URL = "url";

/// Key for format property of the directive payload.
static const std::string FORMAT_KEY = "format";

/// The expected format of the audio in the directive payload.
static const std::string FORMAT = "AUDIO_MPEG";

/// Empty string
static const std::string EMPTY_STRING = "";

std::shared_ptr<TextToSpeechProviderCapabilityAgent> TextToSpeechProviderCapabilityAgent::create(
    std::shared_ptr<aace::engine::textToSpeechProvider::TextToSpeechProviderInterface> textToSpeechProviderEngine,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender) {
    try {
        ThrowIfNull(textToSpeechProviderEngine, "nullTextToSpeechProviderInterface");
        ThrowIfNull(exceptionSender, "nullExceptionSender");
        ThrowIfNull(messageSender, "nullMessageSender");

        auto textToSpeechProviderCapabilityAgent = std::shared_ptr<TextToSpeechProviderCapabilityAgent>(
            new TextToSpeechProviderCapabilityAgent(textToSpeechProviderEngine, exceptionSender, messageSender));

        ThrowIfNull(textToSpeechProviderCapabilityAgent, "nullTextToSpeechProviderCapabilityAgent");
        return textToSpeechProviderCapabilityAgent;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void TextToSpeechProviderCapabilityAgent::handleDirectiveImmediately(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) {
    handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
}

void TextToSpeechProviderCapabilityAgent::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    // Intentional no-op (removeDirective() can only be called from handleDirective() and cancelDirective() functions).
}

void TextToSpeechProviderCapabilityAgent::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    try {
        ThrowIfNot(info && info->directive, "nullDirectiveInfo");
        if (info->directive->getName() == HANDLE_SPEECH.name) {
            handleHandleSpeechDirective(info);
        } else {
            handleUnknownDirective(info);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void TextToSpeechProviderCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    removeDirective(info);
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration TextToSpeechProviderCapabilityAgent::getConfiguration()
    const {
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    auto audioVisualBlockingPolicy = alexaClientSDK::avsCommon::avs::BlockingPolicy(
        alexaClientSDK::avsCommon::avs::BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
    configuration[HANDLE_SPEECH] = audioVisualBlockingPolicy;
    return configuration;
}

TextToSpeechProviderCapabilityAgent::TextToSpeechProviderCapabilityAgent(
    std::shared_ptr<aace::engine::textToSpeechProvider::TextToSpeechProviderInterface> textToSpeechProviderEngine,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender) :
        alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
        alexaClientSDK::avsCommon::utils::RequiresShutdown{"TextToSpeechProviderCapabilityAgent"},
        m_messageSender{messageSender},
        m_textToSpeechProviderEngine{textToSpeechProviderEngine} {
    m_capabilityConfigurations.insert(getTextToSpeechProviderConfiguration());
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> TextToSpeechProviderCapabilityAgent::
    getTextToSpeechProviderConfiguration() {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, PREPARE_SPEECH_CAPABILITY_INTERFACE_TYPE});
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, PREPARE_SPEECH_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY,
                      PREPARE_SPEECH_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

void TextToSpeechProviderCapabilityAgent::doShutdown() {
    m_textToSpeechProviderEngine.reset();
    m_messageSender.reset();
    m_executor.shutdown();
}

void TextToSpeechProviderCapabilityAgent::sendExceptionEncounteredAndReportFailed(
    std::shared_ptr<DirectiveInfo> info,
    const std::string& message,
    alexaClientSDK::avsCommon::avs::ExceptionErrorType type) {
    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), type, message);

    if (info && info->result) {
        info->result->setFailed(message);
    }

    removeDirective(info);
}

void TextToSpeechProviderCapabilityAgent::removeDirective(std::shared_ptr<DirectiveInfo> info) {
    /*
     * Check result too, to catch cases where DirectiveInfo was created locally, without a nullptr result.
     * In those cases there is no messageId to remove because no result was expected.
     */
    if (info->directive && info->result) {
        alexaClientSDK::avsCommon::avs::CapabilityAgent::removeDirective(info->directive->getMessageId());
    }
}

void TextToSpeechProviderCapabilityAgent::setHandlingCompleted(std::shared_ptr<DirectiveInfo> info) {
    if (info && info->result) {
        info->result->setCompleted();
    }

    removeDirective(info);
}

void TextToSpeechProviderCapabilityAgent::handleHandleSpeechDirective(std::shared_ptr<DirectiveInfo> info) {
    auto textToSpeechProviderEngine = m_textToSpeechProviderEngine;
    m_executor.submit([this, info, textToSpeechProviderEngine]() {
        try {
            std::string payload = info->directive->getPayload();
            json result = json::parse(payload);
            auto payloadValid = executeValidatePayload(info, result);
            ThrowIfNot(payloadValid, "invalidHandleSpeechPayload");

            std::string speechId = result.at("speechId");
            std::string urlValue = result.at(URL);
            auto contentIdPosition = urlValue.find(CID_PREFIX);
            std::string contentId = urlValue.substr(contentIdPosition + CID_PREFIX.length());

            auto attachmentReader = info->directive->getAttachmentReader(
                contentId, alexaClientSDK::avsCommon::utils::sds::ReaderPolicy::NONBLOCKING);
            if (!attachmentReader) {
                AACE_ERROR(LX(TAG).d("messageId", info->directive->getMessageId()));
                sendExceptionEncounteredAndReportFailed(
                    info, "attachmentReaderFailed", alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR);
                return;
            }
            auto preparedAudio =
                aace::engine::alexa::AttachmentReaderAudioStream::create(std::move(attachmentReader), nullptr);
            auto metadata = EMPTY_STRING;
            textToSpeechProviderEngine->prepareSpeechCompleted(speechId, preparedAudio, metadata);
            setHandlingCompleted(info);
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Unable to parse payload",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        }
    });
}

void TextToSpeechProviderCapabilityAgent::handleUnknownDirective(std::shared_ptr<DirectiveInfo> info) {
    AACE_ERROR(LX(TAG)
                   .d("reason", "unknownDirective")
                   .d("namespace", info->directive->getNamespace())
                   .d("name", info->directive->getName()));

    m_executor.submit([this, info] {
        sendExceptionEncounteredAndReportFailed(
            info,
            "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName(),
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
    });
}

void TextToSpeechProviderCapabilityAgent::prepareSpeech(
    const std::string& speechId,
    const std::string& text,
    const std::string& assistantId) {
    m_executor.submit([this, speechId, text, assistantId] { executePrepareSpeech(speechId, text, assistantId); });
}

void TextToSpeechProviderCapabilityAgent::executePrepareSpeech(
    const std::string& speechId,
    const std::string& text,
    const std::string& assistantId) {
    try {
        auto type = text.find("<speak>") != std::string::npos ? SSML_TYPE : TEXT_TYPE;
        // clang-format off
        json payload = {
            {"speechId", speechId},
            {"content", {
                {"type", type},
                {"value", text}
            }}
        };
        // clang-format on
        if (!assistantId.empty()) {
            payload["assistantId"] = assistantId;
        }
        auto sythesizeEvent = buildJsonEventString(SYNTHESIZE_EVENT, EMPTY_STRING, payload.dump());
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(sythesizeEvent.second);
        m_messageSender->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool TextToSpeechProviderCapabilityAgent::executeValidatePayload(std::shared_ptr<DirectiveInfo> info, json& result) {
    try {
        if (result.empty()) {
            AACE_ERROR(LX(TAG).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Empty payload",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }

        if (!result.contains("speechId")) {
            AACE_ERROR(LX(TAG).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Speech Id missing",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }

        auto speechId = result.value("speechId", EMPTY_STRING);

        if (speechId.empty()) {
            AACE_ERROR(LX(TAG).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Speech Id empty",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }

        if (!result.contains(FORMAT_KEY)) {
            AACE_ERROR(LX(TAG).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Format missing",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }

        auto formatValue = result.value(FORMAT_KEY, EMPTY_STRING);
        if (formatValue.empty() || formatValue != FORMAT) {
            AACE_ERROR(LX(TAG).d("messageId", info->directive->getMessageId()).d("format", formatValue));
            sendExceptionEncounteredAndReportFailed(
                info,
                "Unknown Format",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }

        if (!result.contains(URL)) {
            AACE_ERROR(LX(TAG).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "URL missing",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }

        auto urlValue = result.value(URL, EMPTY_STRING);
        if (urlValue.empty()) {
            AACE_ERROR(LX(TAG).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info, "URL empty", alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }

        auto contentIdPosition = urlValue.find(CID_PREFIX);
        if (contentIdPosition != 0) {
            AACE_ERROR(LX(TAG).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed(
                info,
                "expected CID url prefix not found",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return false;
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        sendExceptionEncounteredAndReportFailed(
            info,
            "Invalid information",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return false;
    }
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
TextToSpeechProviderCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

}  // namespace textToSpeechProvider
}  // namespace engine
}  // namespace aace
