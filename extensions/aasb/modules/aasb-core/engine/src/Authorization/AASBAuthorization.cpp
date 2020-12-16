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

#include <AASB/Engine/Authorization/AASBAuthorization.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Authorization/Authorization/AuthorizationErrorMessage.h>
#include <AASB/Message/Authorization/Authorization/AuthorizationState.h>
#include <AASB/Message/Authorization/Authorization/AuthorizationStateChangedMessage.h>
#include <AASB/Message/Authorization/Authorization/CancelAuthorizationMessage.h>
#include <AASB/Message/Authorization/Authorization/EventReceivedMessage.h>
#include <AASB/Message/Authorization/Authorization/GetAuthorizationDataMessage.h>
#include <AASB/Message/Authorization/Authorization/GetAuthorizationDataMessageReply.h>
#include <AASB/Message/Authorization/Authorization/LogoutMessage.h>
#include <AASB/Message/Authorization/Authorization/SendEventMessage.h>
#include <AASB/Message/Authorization/Authorization/SetAuthorizationDataMessage.h>
#include <AASB/Message/Authorization/Authorization/StartAuthorizationMessage.h>

namespace aasb {
namespace engine {
namespace authorization {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.authorization.AASBAuthorization");

// Aliases
using Message = aace::engine::aasb::Message;
using AuthorizationState = aace::authorization::Authorization::AuthorizationState;

bool AASBAuthorization::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // Create a weak pointer reference
        std::weak_ptr<AASBAuthorization> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::authorization::authorization::StartAuthorizationMessage::topic(),
            aasb::message::authorization::authorization::StartAuthorizationMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::authorization::authorization::StartAuthorizationMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->startAuthorization(payload.service, payload.data);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::authorization::authorization::CancelAuthorizationMessage::topic(),
            aasb::message::authorization::authorization::CancelAuthorizationMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::authorization::authorization::CancelAuthorizationMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->cancelAuthorization(payload.service);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::authorization::authorization::SendEventMessage::topic(),
            aasb::message::authorization::authorization::SendEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::authorization::authorization::SendEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->sendEvent(payload.service, payload.event);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::authorization::authorization::LogoutMessage::topic(),
            aasb::message::authorization::authorization::LogoutMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::authorization::authorization::LogoutMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->logout(payload.service);
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

std::shared_ptr<AASBAuthorization> AASBAuthorization::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBAuthorization>(new AASBAuthorization());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBAuthorizationFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void AASBAuthorization::eventReceived(const std::string& service, const std::string& event) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::authorization::authorization::EventReceivedMessage message;
        message.payload.service = service;
        message.payload.event = event;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAuthorization::authorizationStateChanged(const std::string& service, AuthorizationState state) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::authorization::authorization::AuthorizationStateChangedMessage message;
        message.payload.service = service;
        message.payload.state = static_cast<aasb::message::authorization::AuthorizationState>(state);

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAuthorization::authorizationError(
    const std::string& service,
    const std::string& error,
    const std::string& errorMessage) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::authorization::authorization::AuthorizationErrorMessage message;
        message.payload.service = service;
        message.payload.error = error;
        message.payload.message = errorMessage;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string AASBAuthorization::getAuthorizationData(const std::string& service, const std::string& key) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::authorization::authorization::GetAuthorizationDataMessage message;
        message.payload.service = service;
        message.payload.key = key;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        if (result.valid()) {
            std::string payload = result.payload();
            aasb::message::authorization::authorization::GetAuthorizationDataMessageReply::Payload replyPayload =
                nlohmann::json::parse(payload);
            AACE_INFO(LX(TAG).m("ReplyReceived"));
            return replyPayload.data;
        } else {
            AACE_ERROR(LX(TAG).d("reason", "Failed to receive response").d("service", service).d("key", key));
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("service", service).d("key", key));
    }

    return "";
}

void AASBAuthorization::setAuthorizationData(
    const std::string& service,
    const std::string& key,
    const std::string& data) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::authorization::authorization::SetAuthorizationDataMessage message;
        message.payload.service = service;
        message.payload.key = key;
        message.payload.data = data;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace authorization
}  // namespace engine
}  // namespace aasb
