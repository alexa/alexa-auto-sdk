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

#ifndef SAMPLEAPP_PROPERTY_MANAGER_PROPERTYMANAGERHANDLER_H
#define SAMPLEAPP_PROPERTY_MANAGER_PROPERTYMANAGERHANDLER_H

#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/PropertyManager/PropertyManager/PropertyStateChangedMessage.h>

#include <unordered_map>
#include <future>
#include <utility>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace propertyManager {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PropertyManagerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class PropertyManagerHandler {
private:
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    PropertyManagerHandler(
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<PropertyManagerHandler> {
        return std::shared_ptr<PropertyManagerHandler>(new PropertyManagerHandler(args...));
    }
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    /**
     * Publishes SetPropertyMessage to notify the Engine to set the property setting.
     */
    bool setProperty(const std::string& name, const std::string& value);

    /**
     * Publishes the GetPropertyMessage to notify the Engine to retrieve the property setting.
     * 
     * A reply message is expected from the Engine. The default wait time for the reply is 1000 ms.
     * If the reply is not received within that time, a timeout occurs.
     *
     * @param [in] requestId A string that will act as a unique identifier for this message
     * @param [in] name The property name
     * @return true if the Engine reply with the property value was received on time, false otherwise
     */
    std::string getProperty(const std::string& name);

private:
    auto subscribeToAASBMessages() -> void;

    /**
     * Handles the PropertyChangedMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePropertyChangedMessage(const std::string& message);

    /**
     * Handles the PropertyStateChangedMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePropertyStateChangedMessage(const std::string& message);

    /**
     * Handles the GetPropertyReplyMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetPropertyReplyMessage(const std::string& message);

    /**
     * Handles the implementation of the PropertyChanged message
     */
    void propertyChanged(const std::string& key, const std::string& newValue);

    /**
     * Handles the implementation of the PropertyStateChanged message
     */
    void propertyStateChanged(
        const std::string& name,
        const std::string& value,
        const aasb::message::propertyManager::propertyManager::PropertyState state);

    using PropertyManagerPromise = std::promise<std::string>;
    std::mutex m_promise_map_access_mutex;
    std::unordered_map<std::string, std::shared_ptr<PropertyManagerPromise>> m_promiseMap;

    std::string waitForAsyncReply(const std::string& messageId);
    void addReplyMessagePromise(const std::string& messageId, std::shared_ptr<PropertyManagerPromise> promise);
    void removeReplyMessagePromise(const std::string& messageId);
    std::shared_ptr<PropertyManagerPromise> getReplyMessagePromise(const std::string& messageId);

private:
    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    std::weak_ptr<View> m_console{};
};

}  // namespace propertyManager
}  // namespace sampleApp

#endif  // SAMPLEAPP_PROPERTY_MANAGER_PROPERTYMANAGERHANDLER_H
