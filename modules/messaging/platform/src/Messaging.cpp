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

#include "AACE/Messaging/Messaging.h"

namespace aace {
namespace messaging {

void Messaging::conversationsReport(const std::string& token, const std::string& conversations) {
    if (m_messagingEngineInterface != nullptr) {
        m_messagingEngineInterface->onConversationsReport(token, conversations);
    }
}

void Messaging::sendMessageFailed(const std::string& token, ErrorCode code, const std::string& message) {
    if (m_messagingEngineInterface != nullptr) {
        m_messagingEngineInterface->onSendMessageFailed(token, code, message);
    }
}

void Messaging::sendMessageSucceeded(const std::string& token) {
    if (m_messagingEngineInterface != nullptr) {
        m_messagingEngineInterface->onSendMessageSucceeded(token);
    }
}

void Messaging::updateMessagesStatusFailed(const std::string& token, ErrorCode code, const std::string& message) {
    if (m_messagingEngineInterface != nullptr) {
        m_messagingEngineInterface->onUpdateMessagesStatusFailed(token, code, message);
    }
}

void Messaging::updateMessagesStatusSucceeded(const std::string& token) {
    if (m_messagingEngineInterface != nullptr) {
        m_messagingEngineInterface->onUpdateMessagesStatusSucceeded(token);
    }
}

void Messaging::updateMessagingEndpointState(
    ConnectionState connectionState,
    PermissionState sendPermission,
    PermissionState readPermission) {
    if (m_messagingEngineInterface != nullptr) {
        m_messagingEngineInterface->onUpdateMessagingEndpointState(connectionState, sendPermission, readPermission);
    }
}

void Messaging::setEngineInterface(std::shared_ptr<MessagingEngineInterface> messagingEngineInterface) {
    m_messagingEngineInterface = messagingEngineInterface;
}

Messaging::~Messaging() = default;

}  // namespace messaging
}  // namespace aace
