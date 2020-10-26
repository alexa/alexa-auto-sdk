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

#include <AASB/Engine/PropertyManager/AASBPropertyManager.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/PropertyManager/PropertyManager/GetPropertyMessage.h>
#include <AASB/Message/PropertyManager/PropertyManager/GetPropertyMessageReply.h>
#include <AASB/Message/PropertyManager/PropertyManager/SetPropertyMessage.h>
#include <AASB/Message/PropertyManager/PropertyManager/PropertyState.h>
#include <AASB/Message/PropertyManager/PropertyManager/PropertyStateChangedMessage.h>
#include <AASB/Message/PropertyManager/PropertyManager/PropertyChangedMessage.h>

namespace aasb {
namespace engine {
namespace propertyManager {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.propertyManager.AASBPropertyManager");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBPropertyManager> AASBPropertyManager::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBPropertyManager>(new AASBPropertyManager());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBPropertyManagerFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBPropertyManager::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;

        std::weak_ptr<AASBPropertyManager> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::propertyManager::propertyManager::SetPropertyMessage::topic(),
            aasb::message::propertyManager::propertyManager::SetPropertyMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::propertyManager::propertyManager::SetPropertyMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->setProperty(payload.name, payload.value);

                    AACE_INFO(LX(TAG, "SetPropertyMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SetPropertyMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::propertyManager::propertyManager::GetPropertyMessage::topic(),
            aasb::message::propertyManager::propertyManager::GetPropertyMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::propertyManager::propertyManager::GetPropertyMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    AACE_INFO(LX(TAG, "GetPropertyMessage").m("MessageRouted"));

                    auto m_messageBroker_lock = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

                    aasb::message::propertyManager::propertyManager::GetPropertyMessageReply getPropertyMessageReply;
                    getPropertyMessageReply.header.messageDescription.replyToId = message.messageId();
                    getPropertyMessageReply.payload.value = sp->getProperty(payload.name);
                    ;
                    getPropertyMessageReply.payload.name = payload.name;
                    m_messageBroker_lock->publish(getPropertyMessageReply.toString()).send();

                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "GetPropertyMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::propertyManager::PropertyManager
//

void AASBPropertyManager::propertyChanged(const std::string& name, const std::string& newValue) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::propertyManager::propertyManager::PropertyChangedMessage message;
        message.payload.name = name;
        message.payload.newValue = newValue;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBPropertyManager::propertyStateChanged(
    const std::string& name,
    const std::string& value,
    const PropertyState state) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::propertyManager::propertyManager::PropertyStateChangedMessage message;
        message.payload.name = name;
        message.payload.value = value;
        message.payload.state = static_cast<aasb::message::propertyManager::PropertyState>(state);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace propertyManager
}  // namespace engine
}  // namespace aasb
