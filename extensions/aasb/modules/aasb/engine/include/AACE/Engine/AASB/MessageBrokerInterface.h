/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_AASB_MESSAGE_BROKER_INTERFACE_H
#define AACE_ENGINE_AASB_MESSAGE_BROKER_INTERFACE_H

#include <string>
#include <memory>
#include <iostream>
#include <functional>
#include <chrono>

#include "PublishMessage.h"

namespace aace {
namespace engine {
namespace aasb {

class MessageBrokerInterface {
public:
    using MessageHandler = std::function<void(const Message& message)>;

    virtual void subscribe(
        const std::string& topic,
        MessageHandler handler,
        Message::Direction direction = Message::Direction::INCOMING) = 0;
    virtual void subscribe(
        const std::string& topic,
        const std::string& action,
        MessageHandler handler,
        Message::Direction direction = Message::Direction::INCOMING) = 0;
    virtual PublishMessage publish(
        const std::string& message,
        Message::Direction direction = Message::Direction::OUTGOING) = 0;
};

}  // namespace aasb
}  // namespace engine
}  // namespace aace

#endif
