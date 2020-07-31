/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <AACE/Messaging/Messaging.h>

namespace sampleApp {

class ApplicationContext;  // forward declare
namespace messaging {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MessagingHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Provides a sample implementation of the @c Messaging platform interface. 
 */
class MessagingHandler : public aace::messaging::Messaging {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    MessagingHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<MessagingHandler> {
        return std::shared_ptr<MessagingHandler>(new MessagingHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::messaging::Messaging interface
    auto sendMessage(const std::string& token, const std::string& message, const std::string& recipients)
        -> void override;
    auto uploadConversations(const std::string& token) -> void override;
    auto updateMessagesStatus(const std::string& token, const std::string& conversationId, const std::string& status)
        -> void override;

private:
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
