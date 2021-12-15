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

#ifndef SAMPLEAPP_MESSAGING_MESSAGINGHANDLER_H
#define SAMPLEAPP_MESSAGING_MESSAGINGHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Messaging/Messaging/ConnectionState.h>
#include <AASB/Message/Messaging/Messaging/ErrorCode.h>
#include <AASB/Message/Messaging/Messaging/PermissionState.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {

class ApplicationContext;  // forward declare
namespace messaging {

using namespace aasb::message::messaging::messaging;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MessagingHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class MessagingHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    MessagingHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<MessagingHandler> {
        return std::shared_ptr<MessagingHandler>(new MessagingHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the SendMessageMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSendMessageMessage(const std::string& message);

    /**
     * Handles the UpdateMessagesStatusMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleUpdateMessagesStatusMessage(const std::string& message);

    /**
     * Handles the UploadConversationsMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleUploadConversationsMessage(const std::string& message);

    /**
     * Send SMS message.
     * 
     * @param [in] token Token id for send message request
     * @param [in] message Body of the SMS text message to be sent
     * @param [in] recipients String in JSON format containing the recipient of the SMS message
     */
    void sendMessage(const std::string& token, const std::string& message, const std::string& recipients);

    /**
     * Update status of SMS messages.
     * 
     * @param [in] token Token id for send message request
     */
    void uploadConversations(const std::string& token);

    /**
     * Upload SMS unread messages message.
     * 
     * @param [in] token Token id for send message request
     */
    void updateMessagesStatus(const std::string& token, const std::string& conversationId, const std::string& status);

    /**
     * Publishes the ConversationsReport message to notify the Engine to upload conversations report to the cloud.
     * 
     * @param [in] token Token id for send message request
     * @param [in] conversations String in JSON format representing all conversations with unread SMS messages
     */
    void conversationsReport(const std::string& token, const std::string& conversations);

    /**
     * Publishes the SendMessageFailed message to notify the Engine the message send failed.
     * 
     * @param [in] token Token id for send message request
     * @param [in] code The error code identifying the failure
     * @param [in] message The message explaining the error
     */
    void sendMessageFailed(const std::string& token, ErrorCode code, const std::string& message = "");

    /**
     * Publishes the SendMessageSuceeded message to notify the Engine that message send was successful.
     * 
     * @param [in] token Token id for send message request
     */
    void sendMessageSucceeded(const std::string& token);

    /**
     * Publishes the UpdateMessagesStatusFailed message to notify the Engine the message status update failed.
     * 
     * @param [in] token Token id for send message request
     * @param [in] code The error code identifying the failure
     * @param [in] message The message explaining the error
     */
    void updateMessagesStatusFailed(const std::string& token, ErrorCode code, const std::string& message = "");

    /**
     * Publishes the UpdateMessagesStatusSucceeded message to notify the Engine the message status update suceeded.
     * 
     * @param [in] token Token id for send message request
     */
    void updateMessagesStatusSucceeded(const std::string& token);

    /**
     * Publishes the UpdateMessagingEndpointState message to notify the Engine of updates to the messaging endpoint state.
     * 
     * @param [in] connectionState The value for the connection state
     * @param [in] sendPermission The value for the send permission
     * @param [in] readPermission The value for the read permission
     */
    void updateMessagingEndpointState(
        ConnectionState connectionState,
        PermissionState sendPermission,
        PermissionState readPermission);

    /**
     * Validate that conversations report has the correct fields.
     * @return @true if valid, else @false if it contains errors.
     */
    auto validateConversationsReport(std::stringstream& buffer) -> bool;

    /**
     * Updates the status of the messages from the currently loaded conversations report.
     */
    auto updateMessages(const std::string& conversationId, const json& statusJson) -> void;

    /**
     * Show message to console. 
     */
    auto showMessage(const std::string& message, bool asError = false) -> void;

    /**
     * Reset state global variables.
     */
    auto resetGlobals() -> void;

    /**
     * Log entry.
     */
    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;

    /**
     * Setup all the event listeners.
     */
    auto setupUI() -> void;

    auto subscribeToAASBMessages() -> void;

private:
    /// Application context
    std::shared_ptr<ApplicationContext> m_applicationContext{};

    /// Console
    std::weak_ptr<View> m_console{};

    /// Read messages
    std::vector<std::string> m_readMessages;

    /// Send messages
    std::vector<std::string> m_sentMessages;

    /// Automatic responses
    bool m_automaticResponses;

    /// Conversations list
    json m_conversations;

    /// Update status
    json m_status;

    /// Conversation id
    std::string m_conversationId;

    /// Token for current request
    std::string m_token;

    /// Connection state
    ConnectionState m_connectionState;

    /// Send permission state
    PermissionState m_sendPermission;

    /// Read permission state
    PermissionState m_readPermission;
};

}  // namespace messaging
}  // namespace sampleApp

#endif  // SAMPLEAPP_MESSAGING_MESSAGING_H
