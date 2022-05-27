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

#ifndef AACE_ENGINE_MESSAGE_BROKER_PUBLISH_MESSAGE_H
#define AACE_ENGINE_MESSAGE_BROKER_PUBLISH_MESSAGE_H

#include <string>
#include <memory>
#include <iostream>
#include <functional>
#include <chrono>

#include "Message.h"

namespace aace {
namespace engine {
namespace messageBroker {

class PublishMessage {
public:
    using SuccessHandler = std::function<void(const Message& message)>;
    using ErrorHandler = std::function<void()>;
    using InvokeHandler = std::function<Message(const PublishMessage& pm, bool sync)>;

    PublishMessage(
        Message::Direction direction,
        const std::string& message,
        std::chrono::milliseconds timeout,
        InvokeHandler invokeHandler);
    PublishMessage(const PublishMessage& pm);

    PublishMessage& timeout(std::chrono::milliseconds duration);
    PublishMessage& success(SuccessHandler handler);
    PublishMessage& error(ErrorHandler handler);

    void send();
    Message get();

    // accessor methods
    Message::Direction direction() const;
    std::string msg() const;
    std::chrono::milliseconds timeout() const;
    SuccessHandler successHandler() const;
    ErrorHandler errorHandler() const;

    Message message() const;
    bool valid() const;

protected:
    Message::Direction m_direction;
    std::string m_message;
    std::chrono::milliseconds m_timeout;
    InvokeHandler m_invokeHandler;
    SuccessHandler m_successHandler;
    ErrorHandler m_errorHandler;

    // keep track of whether or not the message was sent
    bool m_messageSent = false;
};

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace

#endif
