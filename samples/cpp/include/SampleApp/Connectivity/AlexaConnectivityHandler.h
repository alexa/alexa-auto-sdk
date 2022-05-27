/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_CONNECTIVITY_ALEXA_CONNECTIVITY_HANDLER_H
#define SAMPLEAPP_CONNECTIVITY_ALEXA_CONNECTIVITY_HANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/ApplicationContext.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AASB/Message/Connectivity/AlexaConnectivity/StatusCode.h>

#include <AACE/Core/MessageBroker.h>

#include <unordered_map>
#include <future>
#include <utility>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace connectivity {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaConnectivityHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AlexaConnectivityHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    AlexaConnectivityHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AlexaConnectivityHandler> {
        return std::shared_ptr<AlexaConnectivityHandler>(new AlexaConnectivityHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the GetConnectivityStateMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetConnectivityStateMessage(const std::string& message);

    /**
     * Handles the GetIdentifierMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetIdentifierMessage(const std::string& message);

    /**
     * Handles the ConnectivityStateChangeReplyMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleConnectivityStateChangeReplyMessage(const std::string& message);

    /**
     * Handles the ConnectivityStateChangeReplyMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSendConnectivityEventReplyMessage(const std::string& message);

    /** 
     * Retrieves the connectivity state from the platform implementation.
     * 
     * @return A @c std::string representing the connectivity state in structured JSON format
     */
    std::string getConnectivityState();

    /** 
     * Retrieves the identifier from the platform implementation.
     * 
     * @return A @c std::string representing the network identifier, or empty for default behavior
     */
    std::string getIdentifier();

    /**
     * Provides the status of sendConnectivityEvent.
     * 
     * @param [in] token The Id of the message that this one is replying to
     * @param [in] statusCode represents the delivery status of event sent using SendConnectivityEventMessage
     */
    void connectivityEventResponse(
        const std::string& token,
        aasb::message::connectivity::alexaConnectivity::StatusCode statusCode);

    /**
     * Publishes the ConnectivityStateChange message to notify the Engine of a change in the connectivity state.
     * The Engine calls getConnectivityState to retrieve the connectivity state and communicate any changes to Alexa.
     *
     * A reply message is expected from the Engine. The default wait time for the reply is 1000 ms.
     * If the reply is not received within that time, a timeout occurs.
     *
     * @return true if connectivity state was processed successfully, false otherwise.
     */
    bool connectivityStateChange();

    /**
     * Notifies an event in the connectivity to the Engine.
     * 
     * A reply message is expected from the Engine. The default wait time for the reply is 1000 ms.
     * If the reply is not received within that time, a timeout occurs.
     * 
     * @param [in] event The stringified JSON containing the event.
     */
    aasb::message::connectivity::alexaConnectivity::StatusCode sendConnectivityEvent(const std::string& event);

private:
    using AlexaConnectivityPromise = std::promise<json>;
    std::mutex m_promise_map_access_mutex;
    std::unordered_map<std::string, std::shared_ptr<AlexaConnectivityPromise>> m_promiseMap;

    json waitForAsyncReply(const std::string& messageId);
    void addReplyMessagePromise(const std::string& messageId, std::shared_ptr<AlexaConnectivityPromise> promise);
    void removeReplyMessagePromise(const std::string& messageId);
    std::shared_ptr<AlexaConnectivityPromise> getReplyMessagePromise(const std::string& messageId);

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;

    /// Current Connectivity State
    std::string m_connectivityState;
};

}  // namespace connectivity
}  // namespace sampleApp

#endif  // SAMPLEAPP_CONNECTIVITY_ALEXA_CONNECTIVITY_HANDLER_H
