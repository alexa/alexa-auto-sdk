/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/Wakeword/AASBWakeword.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/wakeword/wakeword/OnWakewordDetectedMessage.h>
#include <AASB/Message/wakeword/wakeword/SetWakewordStatusMessage.h>

namespace aasb {
namespace engine {
namespace wakeword {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.Wakeword.AASBWakeword");

// aliases
using Message = aace::engine::messageBroker::Message;

std::shared_ptr<AASBWakeword> AASBWakeword::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBWakeword>(new AASBWakeword());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBWakewordFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBWakeword::initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;

        std::weak_ptr<AASBWakeword> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::wakeword::wakeword::SetWakewordStatusMessage::topic(),
            aasb::message::wakeword::wakeword::SetWakewordStatusMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::wakeword::wakeword::SetWakewordStatusMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    bool success = sp->enable3PWakeword(payload.name, payload.value);

                    // send SetWakewordstatus  reply
                    auto m_messageBroker_lock = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

                    aasb::message::wakeword::wakeword::SetWakewordStatusMessageReply setWakewordStatusMessageReply;
                    setWakewordStatusMessageReply.header.messageDescription.replyToId = message.messageId();
                    setWakewordStatusMessageReply.payload.success = success;
                    m_messageBroker_lock->publish(setWakewordStatusMessageReply.toString()).send();

                    AACE_INFO(LX(TAG).m("MessageRouted"));
                	
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG).d("reason", ex.what()));
                }
            });


        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AASBWakeword::onWakewordDetected(const std::string& wakeword, WakeWordIndex beginIndex, WakeWordIndex endIndex) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::wakeword::wakeword::OnWakewordDetectedMessage message;
        message.payload.wakeword = wakeword;
        message.payload.beginIndex = beginIndex;
        message.payload.endIndex = endIndex;

        m_messageBroker_lock->publish(message.toString()).send();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace wakeword
}  // namespace engine
}  // namespace aasb
