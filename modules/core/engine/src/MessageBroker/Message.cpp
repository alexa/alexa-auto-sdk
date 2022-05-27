/*
 * Copyright 2017-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/MessageBroker/Message.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/UUID/UUID.h>
#include <AACE/Engine/Utils/String/StringUtils.h>

namespace aace {
namespace engine {
namespace messageBroker {

// String to identify log entries originating from this file.
static const std::string TAG("aace.messageBroker.Message");

// symbolic constants
const Message Message::INVALID = Message();

Message::Message() : m_direction(Direction::OUTGOING) {
}

Message::Message(const std::string& msg, Direction direction) {
    try {
        m_message = nlohmann::json::parse(msg);
        ThrowIf(m_message.is_null(), "invalidMessage");

        auto messageType = m_message["/header/messageType"_json_pointer];
        ThrowIfNull(messageType, "missingMessageType");

        auto messageId = m_message["/header/id"_json_pointer];
        ThrowIfNull(messageId, "missingMessageId");

        m_messageId = messageId;

        if (aace::engine::utils::string::equal(messageType.get<std::string>(), "publish", false)) {
            m_messageType = MessageType::PUBLISH;

            auto topic = m_message["/header/messageDescription/topic"_json_pointer];
            ThrowIfNull(topic, "missingMessageTopic");

            auto action = m_message["/header/messageDescription/action"_json_pointer];
            ThrowIfNull(action, "missingMessageAction");

            m_topic = topic;
            m_action = action;
        } else if (aace::engine::utils::string::equal(messageType.get<std::string>(), "reply", false)) {
            m_messageType = MessageType::REPLY;

            auto replyTo = m_message["/header/messageDescription/replyToId"_json_pointer];
            ThrowIfNull(replyTo, "missingReplyTo");

            auto topic = m_message["/header/messageDescription/topic"_json_pointer];
            ThrowIfNull(topic, "missingMessageTopic");

            auto action = m_message["/header/messageDescription/action"_json_pointer];
            ThrowIfNull(action, "missingMessageAction");

            m_replyTo = replyTo;
            m_topic = topic;
            m_action = action;
        } else {
            Throw("invalidMessageType");
        }

        m_direction = direction;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("msg", msg));
        m_message = nullptr;
    }
}

bool Message::valid() const {
    return !m_message.is_null();
}

const std::string& Message::messageId() const {
    return m_messageId;
}

Message::MessageType Message::messageType() const {
    return m_messageType;
}

const std::string& Message::topic() const {
    return m_topic;
}

const std::string& Message::action() const {
    return m_action;
}

const std::string& Message::replyTo() const {
    return m_replyTo;
}

std::string Message::payload() const {
    try {
        auto payloadIt = m_message.find("payload");
        ThrowIf(payloadIt == m_message.end(), "missingPayloadInMessage");
        ThrowIfNot(payloadIt->is_object(), "invalidPayloadType");

        return payloadIt->get<nlohmann::json>().dump(3);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::string();
    }
}

Message::Direction Message::direction() const {
    return m_direction;
}

Message::Direction Message::replyDirection() const {
    return m_direction == Direction::INCOMING ? Direction::OUTGOING : Direction::INCOMING;
}

std::string Message::str() const {
    return m_message.dump(3);
}

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace
