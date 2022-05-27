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

#ifndef AACE_ENGINE_MESSAGE_BROKER_MESSAGE_H
#define AACE_ENGINE_MESSAGE_BROKER_MESSAGE_H

#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace messageBroker {

class Message {
private:
    Message();

public:
    enum class Direction { INCOMING, OUTGOING };

    enum class MessageType { PUBLISH, REPLY };

    Message(const std::string& msg, Direction direction);

    bool valid() const;

    Direction direction() const;
    Direction replyDirection() const;

    // header
    const std::string& messageId() const;
    MessageType messageType() const;

    // message description
    const std::string& topic() const;
    const std::string& action() const;
    const std::string& replyTo() const;

    // payload
    std::string payload() const;

    // serialize
    std::string str() const;

    // symbolic constants
    static const Message INVALID;

private:
    nlohmann::json m_message;
    Direction m_direction;
    MessageType m_messageType;
    std::string m_messageId;
    std::string m_topic;
    std::string m_action;
    std::string m_replyTo;
    std::string m_payload;
};

inline std::ostream& operator<<(std::ostream& stream, const Message& message) {
    stream << message.str();
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const Message::Direction& direction) {
    switch (direction) {
        case Message::Direction::INCOMING:
            stream << "INCOMING";
            break;
        case Message::Direction::OUTGOING:
            stream << "OUTGOING";
            break;
        default:
            stream << "INVALID";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const Message::MessageType& messageType) {
    switch (messageType) {
        case Message::MessageType::PUBLISH:
            stream << "PUBLISH";
            break;
        case Message::MessageType::REPLY:
            stream << "REPLY";
            break;
        default:
            stream << "INVALID";
            break;
    }
    return stream;
}

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace

#endif
