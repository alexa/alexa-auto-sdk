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

#include <AASB/Engine/Messaging/AASBMessaging.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Messaging/Messaging/ConnectionState.h>
#include <AASB/Message/Messaging/Messaging/ConversationsReportMessage.h>
#include <AASB/Message/Messaging/Messaging/ErrorCode.h>
#include <AASB/Message/Messaging/Messaging/PermissionState.h>
#include <AASB/Message/Messaging/Messaging/SendMessageFailedMessage.h>
#include <AASB/Message/Messaging/Messaging/SendMessageMessage.h>
#include <AASB/Message/Messaging/Messaging/SendMessageSucceededMessage.h>
#include <AASB/Message/Messaging/Messaging/UpdateMessagesStatusFailedMessage.h>
#include <AASB/Message/Messaging/Messaging/UpdateMessagesStatusMessage.h>
#include <AASB/Message/Messaging/Messaging/UpdateMessagesStatusSucceededMessage.h>
#include <AASB/Message/Messaging/Messaging/UpdateMessagingEndpointStateMessage.h>
#include <AASB/Message/Messaging/Messaging/UploadConversationsMessage.h>

#include <future>

namespace aasb {
namespace engine {
namespace messaging {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.messaging.AASBMessaging");

// Aliases
using Message = aace::engine::aasb::Message;
using ConnectionState = aace::messaging::Messaging::ConnectionState;
using PermissionState = aace::messaging::Messaging::PermissionState;

bool AASBMessaging::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // Create a weak pointer reference
        std::weak_ptr<AASBMessaging> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::messaging::messaging::ConversationsReportMessage::topic(),
            aasb::message::messaging::messaging::ConversationsReportMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::messaging::messaging::ConversationsReportMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->conversationsReport(payload.token, payload.conversations);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::messaging::messaging::SendMessageFailedMessage::topic(),
            aasb::message::messaging::messaging::SendMessageFailedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::messaging::messaging::SendMessageFailedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->sendMessageFailed(payload.token, static_cast<ErrorCode>(payload.code), payload.message);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::messaging::messaging::SendMessageSucceededMessage::topic(),
            aasb::message::messaging::messaging::SendMessageSucceededMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::messaging::messaging::SendMessageSucceededMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->sendMessageSucceeded(payload.token);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::messaging::messaging::UpdateMessagesStatusFailedMessage::topic(),
            aasb::message::messaging::messaging::UpdateMessagesStatusFailedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::messaging::messaging::UpdateMessagesStatusFailedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->updateMessagesStatusFailed(
                        payload.token, static_cast<ErrorCode>(payload.code), payload.message);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::messaging::messaging::UpdateMessagesStatusSucceededMessage::topic(),
            aasb::message::messaging::messaging::UpdateMessagesStatusSucceededMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::messaging::messaging::UpdateMessagesStatusSucceededMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->updateMessagesStatusSucceeded(payload.token);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::messaging::messaging::UpdateMessagingEndpointStateMessage::topic(),
            aasb::message::messaging::messaging::UpdateMessagingEndpointStateMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::messaging::messaging::UpdateMessagingEndpointStateMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->updateMessagingEndpointState(
                        static_cast<ConnectionState>(payload.connectionState),
                        static_cast<PermissionState>(payload.sendPermission),
                        static_cast<PermissionState>(payload.readPermission));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AASBMessaging> AASBMessaging::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBMessaging>(new AASBMessaging());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBMessagingFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void AASBMessaging::sendMessage(const std::string& token, const std::string& text, const std::string& recipients) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::messaging::messaging::SendMessageMessage message;
        message.payload.token = token;
        message.payload.message = text;
        message.payload.recipients = recipients;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBMessaging::updateMessagesStatus(
    const std::string& token,
    const std::string& conversationId,
    const std::string& status) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::messaging::messaging::UpdateMessagesStatusMessage message;
        message.payload.token = token;
        message.payload.conversationId = conversationId;
        message.payload.status = status;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBMessaging::uploadConversations(const std::string& token) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::messaging::messaging::UploadConversationsMessage message;
        message.payload.token = token;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace messaging
}  // namespace engine
}  // namespace aasb
