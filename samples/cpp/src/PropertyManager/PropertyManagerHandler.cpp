/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/ApplicationContext.h"
#include "SampleApp/PropertyManager/PropertyManagerHandler.h"

#include "AACE/Engine/Core/EngineMacros.h"

#include <AASB/Message/PropertyManager/PropertyManager/GetPropertyMessage.h>
#include <AASB/Message/PropertyManager/PropertyManager/PropertyChangedMessage.h>
#include <AASB/Message/PropertyManager/PropertyManager/SetPropertyMessage.h>

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace propertyManager {

using namespace aasb::message::propertyManager::propertyManager;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PropertyManagerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using MessageBroker = aace::core::MessageBroker;

static const uint32_t ASYNC_REPLY_TIMEOUT = 1000;

PropertyManagerHandler::PropertyManagerHandler(
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_loggerHandler{std::move(loggerHandler)}, m_messageBroker{std::move(messageBroker)} {
    subscribeToAASBMessages();
}

std::weak_ptr<logger::LoggerHandler> PropertyManagerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void PropertyManagerHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to PropertyChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePropertyChangedMessage(message); },
        PropertyChangedMessage::topic(),
        PropertyChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to PropertyStateChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePropertyStateChangedMessage(message); },
        PropertyStateChangedMessage::topic(),
        PropertyStateChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetPropertyReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetPropertyReplyMessage(message); },
        GetPropertyMessage::topic(),
        GetPropertyMessage::action());
}

void PropertyManagerHandler::handlePropertyChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received PropertyChangedMessage");
    PropertyChangedMessage msg = json::parse(message);
    propertyChanged(msg.payload.name, msg.payload.newValue);
}

void PropertyManagerHandler::handlePropertyStateChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received PropertyStateChangedMessage");
    PropertyStateChangedMessage msg = json::parse(message);
    propertyStateChanged(msg.payload.name, msg.payload.value, msg.payload.state);
}

void PropertyManagerHandler::handleGetPropertyReplyMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetPropertyReplyMessage");
    GetPropertyMessageReply msg = json::parse(message);

    auto promise = getReplyMessagePromise(msg.header.messageDescription.replyToId);
    ThrowIfNull(promise, "invalidPromise");
    promise->set_value(msg.payload.value);
}

bool PropertyManagerHandler::setProperty(const std::string& name, const std::string& value) {
    // Publish the "SetProperty" message
    SetPropertyMessage msg;
    msg.payload.name = name;
    msg.payload.value = value;

    m_messageBroker->publish(msg.toString());

    // True is returned since the SetPropertyMesssage is always published.
    // PropertyStateChangedMessage will notify if the property change was a success or a failure.
    return true;
}

std::string PropertyManagerHandler::getProperty(const std::string& name) {
    GetPropertyMessage msg;
    msg.payload.name = name;
    m_messageBroker->publish(msg.toString());
    return waitForAsyncReply(msg.header.id);
}

void PropertyManagerHandler::propertyChanged(const std::string& key, const std::string& newValue) {
    std::stringstream ss;
    ss << "propertyChanged:key=" << key << ",new value=" << newValue;
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

void PropertyManagerHandler::propertyStateChanged(
    const std::string& name,
    const std::string& value,
    aasb::message::propertyManager::propertyManager::PropertyState state) {
    std::stringstream ss;
    ss << "propertyStateChanged:name=" << name << ",value=" << value << ",state=";
    switch (state) {
        case PropertyState::SUCCEEDED:
            ss << "SUCCEEDED";
            break;
        case PropertyState::FAILED:
            ss << "FAILED";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "Invalid PropertyState value");
            return;
    }
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

void PropertyManagerHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "PropertyManagerHandler", message);
}

std::string PropertyManagerHandler::waitForAsyncReply(const std::string& messageId) {
    // create the promise for the property manager reply message to fulfill
    std::shared_ptr<PropertyManagerPromise> promise = std::make_shared<PropertyManagerPromise>();

    // create a future to receive the promised property manager reply message when it is received
    std::shared_future<std::string> future(promise->get_future());

    std::string success = "";
    try {
        addReplyMessagePromise(messageId, promise);
        ThrowIfNot(
            future.wait_for(std::chrono::milliseconds(ASYNC_REPLY_TIMEOUT)) == std::future_status::ready,
            "replyMessageTimeout:id=" + messageId);
        ThrowIfNot(future.valid(), "invalidMessageResponse");
        success = future.get();
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "waitForAsyncReply " + std::string(ex.what()));
        promise->set_exception(std::current_exception());
    }
    removeReplyMessagePromise(messageId);
    return success;
}

void PropertyManagerHandler::addReplyMessagePromise(
    const std::string& messageId,
    std::shared_ptr<PropertyManagerPromise> promise) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        ThrowIf(m_promiseMap.find(messageId) != m_promiseMap.end(), "messageIdAlreadyExists");

        // add the promise to the promise map
        m_promiseMap[messageId] = promise;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "addReplyMessagePromise " + std::string(ex.what()));
    }
}

void PropertyManagerHandler::removeReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        // remove the promise from the promise map
        m_promiseMap.erase(messageId);
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "removeReplyMessagePromise " + std::string(ex.what()));
    }
}

std::shared_ptr<PropertyManagerHandler::PropertyManagerPromise> PropertyManagerHandler::getReplyMessagePromise(
    const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        return it->second;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "getReplyMessagePromise " + std::string(ex.what()));
        return nullptr;
    }
}

}  // namespace propertyManager
}  // namespace sampleApp
