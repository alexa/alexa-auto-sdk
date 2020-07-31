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

#include "AACE/Engine/Messaging/MessagingEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace aace {
namespace engine {
namespace messaging {

// String to identify log entries originating from this file.
static const std::string TAG("aace.messaging.MessagingEngineImpl");

using namespace alexaClientSDK::capabilityAgents::messaging;

MessagingEngineImpl::MessagingEngineImpl(std::shared_ptr<aace::messaging::Messaging> messagingPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_messagingPlatformInterface(messagingPlatformInterface) {
}

bool MessagingEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender) {
    try {
        AACE_INFO(LX(TAG).m("Creating messaging capability agent"));

        // Create instance of messaging capability agent
        m_messagingCapabilityAgent = MessagingCapabilityAgent::create(exceptionSender, contextManager, messageSender);
        ThrowIfNull(m_messagingCapabilityAgent, "couldNotCreateCapabilityAgent");

        // Register as observer for SMS directives
        m_messagingCapabilityAgent->addObserver(shared_from_this());

        // Register capability with the default endpoint
        defaultEndpointBuilder->withCapability(m_messagingCapabilityAgent, m_messagingCapabilityAgent);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<MessagingEngineImpl> MessagingEngineImpl::create(
    std::shared_ptr<aace::messaging::Messaging> messagingPlatformInterface,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender) {
    try {
        ThrowIfNull(messagingPlatformInterface, "nullPlatformInterface");
        ThrowIfNull(defaultEndpointBuilder, "nullDefaultEndpointBuilder");
        ThrowIfNull(contextManager, "nullContextManager");
        ThrowIfNull(exceptionSender, "nullExceptionSender");
        ThrowIfNull(messageSender, "nullMessageSender");

        auto messagingEngineImpl =
            std::shared_ptr<MessagingEngineImpl>(new MessagingEngineImpl(messagingPlatformInterface));

        ThrowIfNot(
            messagingEngineImpl->initialize(defaultEndpointBuilder, exceptionSender, contextManager, messageSender),
            "initializeMessagingEngineImplFailed");

        // set the platform engine interface reference
        messagingPlatformInterface->setEngineInterface(messagingEngineImpl);

        return messagingEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void MessagingEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));

    if (m_messagingCapabilityAgent != nullptr) {
        m_messagingCapabilityAgent->shutdown();
        m_messagingCapabilityAgent.reset();
    }

    if (m_messagingPlatformInterface != nullptr) {
        m_messagingPlatformInterface->setEngineInterface(nullptr);
        m_messagingPlatformInterface.reset();
    }
}

void MessagingEngineImpl::sendMessage(
    const std::string& token,
    MessagingEndpoint endpoint,
    const std::string& payload) {
    AACE_INFO(LX(TAG).sensitive("payload", payload));
    if (m_messagingPlatformInterface != nullptr) {
        try {
            auto messageJson = json::parse(payload);
            if (messageJson.find("messagePayload") != messageJson.end()) {
                auto messagePayload = messageJson["messagePayload"];
                if (messagePayload.find("text") != messagePayload.end() && messagePayload["text"].is_string()) {
                    auto message = messagePayload["text"].get<std::string>();
                    if (messageJson.find("recipients") != messageJson.end()) {
                        auto recipients = messageJson["recipients"].dump();
                        m_messagingPlatformInterface->sendMessage(token, message, recipients);
                    } else {
                        AACE_ERROR(LX(TAG).d("missingRecipients", payload));
                    }
                } else {
                    AACE_ERROR(LX(TAG).d("missingMessagePayloadText", payload));
                }
            } else {
                AACE_ERROR(LX(TAG).d("missingMessagePayload", payload));
            }
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    }
}

void MessagingEngineImpl::updateMessagesStatus(
    const std::string& token,
    MessagingEndpoint endpoint,
    const std::string& payload) {
    AACE_INFO(LX(TAG).sensitive("payload", payload));
    if (m_messagingPlatformInterface != nullptr) {
        try {
            auto messageJson = json::parse(payload);
            if (messageJson.find("conversationId") != messageJson.end() && messageJson["conversationId"].is_string()) {
                auto conversationId = messageJson["conversationId"].get<std::string>();
                if (messageJson.find("statusMap") != messageJson.end()) {
                    auto statusMap = messageJson["statusMap"].dump();
                    m_messagingPlatformInterface->updateMessagesStatus(token, conversationId, statusMap);
                } else {
                    AACE_ERROR(LX(TAG).d("missingStatusMap", payload));
                }
            } else {
                AACE_ERROR(LX(TAG).d("missingConversationId", payload));
            }
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    }
}

void MessagingEngineImpl::uploadConversations(
    const std::string& token,
    MessagingEndpoint endpoint,
    const std::string& payload) {
    AACE_INFO(LX(TAG).sensitive("payload", payload));
    if (m_messagingPlatformInterface != nullptr) {
        m_messagingPlatformInterface->uploadConversations(token);
    }
}

void MessagingEngineImpl::onConversationsReport(const std::string& token, const std::string& conversations) {
    if (m_messagingCapabilityAgent != nullptr) {
        AACE_INFO(LX(TAG).d("token", token).sensitive("conversations", conversations));
        m_messagingCapabilityAgent->conversationsReport(token, conversations);
    }
}

void MessagingEngineImpl::onSendMessageFailed(const std::string& token, ErrorCode code, const std::string& message) {
    if (m_messagingCapabilityAgent != nullptr) {
        AACE_INFO(LX(TAG).d("token", token).d("code", static_cast<int>(code)).d("message", message));
        m_messagingCapabilityAgent->sendMessageFailed(token, convertErrorCode(code), message);
    }
}

void MessagingEngineImpl::onSendMessageSucceeded(const std::string& token) {
    if (m_messagingCapabilityAgent != nullptr) {
        AACE_INFO(LX(TAG).d("token", token));
        m_messagingCapabilityAgent->sendMessageSucceeded(token);
    }
}

void MessagingEngineImpl::onUpdateMessagesStatusFailed(
    const std::string& token,
    ErrorCode code,
    const std::string& message) {
    if (m_messagingCapabilityAgent != nullptr) {
        AACE_INFO(LX(TAG).d("token", token).d("code", static_cast<int>(code)).d("message", message));
        m_messagingCapabilityAgent->updateMessagesStatusFailed(token, convertErrorCode(code), message);
    }
}

void MessagingEngineImpl::onUpdateMessagesStatusSucceeded(const std::string& token) {
    if (m_messagingCapabilityAgent != nullptr) {
        AACE_INFO(LX(TAG).d("token", token));
        m_messagingCapabilityAgent->updateMessagesStatusSucceeded(token);
    }
}

void MessagingEngineImpl::onUpdateMessagingEndpointState(
    ConnectionState connectionState,
    PermissionState sendPermission,
    PermissionState readPermission) {
    if (m_messagingCapabilityAgent != nullptr) {
        AACE_INFO(LX(TAG)
                      .d("connectionState", static_cast<int>(connectionState))
                      .d("sendPermission", static_cast<int>(sendPermission))
                      .d("readPermission", static_cast<int>(readPermission)));
        auto messagingEndpointState = MessagingCapabilityAgent::MessagingEndpointState(
            convertConnectionState(connectionState),
            convertPermissionState(sendPermission),
            convertPermissionState(readPermission));
        m_messagingCapabilityAgent->updateMessagingEndpointState(messagingEndpointState, MessagingEndpoint::DEFAULT);
    }
}

MessagingCapabilityAgent::StatusErrorCode MessagingEngineImpl::convertErrorCode(ErrorCode code) {
    switch (code) {
        case ErrorCode::GENERIC_FAILURE:
            return MessagingCapabilityAgent::StatusErrorCode::GENERIC_FAILURE;
        case ErrorCode::NO_CONNECTIVITY:
            return MessagingCapabilityAgent::StatusErrorCode::NO_CONNECTIVITY;
        case ErrorCode::NO_PERMISSION:
            return MessagingCapabilityAgent::StatusErrorCode::NO_PERMISSION;
    }

    return MessagingCapabilityAgent::StatusErrorCode::GENERIC_FAILURE;
}

MessagingCapabilityAgent::ConnectionState MessagingEngineImpl::convertConnectionState(ConnectionState state) {
    switch (state) {
        case ConnectionState::CONNECTED:
            return MessagingCapabilityAgent::ConnectionState::CONNECTED;
        case ConnectionState::DISCONNECTED:
            return MessagingCapabilityAgent::ConnectionState::DISCONNECTED;
    }

    return MessagingCapabilityAgent::ConnectionState::DISCONNECTED;
}

MessagingCapabilityAgent::PermissionState MessagingEngineImpl::convertPermissionState(PermissionState permission) {
    switch (permission) {
        case PermissionState::ON:
            return MessagingCapabilityAgent::PermissionState::ON;
        case PermissionState::OFF:
            return MessagingCapabilityAgent::PermissionState::OFF;
    }

    return MessagingCapabilityAgent::PermissionState::OFF;
}

}  // namespace messaging
}  // namespace engine
}  // namespace aace
