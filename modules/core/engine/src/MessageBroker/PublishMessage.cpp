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

#include <AACE/Engine/MessageBroker/PublishMessage.h>
#include <AACE/Engine/MessageBroker/MessageBrokerEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace messageBroker {

// String to identify log entries originating from this file.
static const std::string TAG("aace.messageBroker.PublishMessage");

PublishMessage::PublishMessage(
    Message::Direction direction,
    const std::string& message,
    std::chrono::milliseconds timeout,
    InvokeHandler invokeHandler) :
        m_direction(direction), m_message(message), m_timeout(timeout), m_invokeHandler(invokeHandler) {
}

PublishMessage::PublishMessage(const PublishMessage& pm) {
    m_direction = pm.m_direction;
    m_message = pm.m_message;
    m_timeout = pm.m_timeout;
    m_successHandler = pm.m_successHandler;
    m_errorHandler = pm.m_errorHandler;
    m_invokeHandler = pm.m_invokeHandler;
}

PublishMessage& PublishMessage::timeout(std::chrono::milliseconds value) {
    m_timeout = value;
    return *this;
}

PublishMessage& PublishMessage::success(SuccessHandler handler) {
    m_successHandler = handler;
    return *this;
}

PublishMessage& PublishMessage::error(ErrorHandler handler) {
    m_errorHandler = handler;
    return *this;
}

void PublishMessage::send() {
    try {
        m_messageSent = true;

        ThrowIfNull(m_invokeHandler, "invalidPublishMessageInvokeHandler");

        m_invokeHandler(*this, false);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "send").d("reason", ex.what()));
    }
}

Message PublishMessage::get() {
    try {
        m_messageSent = true;

        ThrowIfNull(m_invokeHandler, "invalidPublishMessageInvokeHandler");

        return m_invokeHandler(*this, true);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "get").d("reason", ex.what()));
        return Message::INVALID;
    }
}

std::string PublishMessage::msg() const {
    return m_message;
}

Message::Direction PublishMessage::direction() const {
    return m_direction;
}

std::chrono::milliseconds PublishMessage::timeout() const {
    return m_timeout;
}

PublishMessage::SuccessHandler PublishMessage::successHandler() const {
    return m_successHandler;
}

PublishMessage::ErrorHandler PublishMessage::errorHandler() const {
    return m_errorHandler;
}

Message PublishMessage::message() const {
    return Message(m_message, m_direction);
}

bool PublishMessage::valid() const {
    return m_invokeHandler != nullptr && m_message.empty() == false;
}

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace
