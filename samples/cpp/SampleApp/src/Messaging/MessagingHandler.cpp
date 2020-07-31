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

#include "SampleApp/ApplicationContext.h"
#include "SampleApp/Messaging/MessagingHandler.h"

// C++ Standard Library
#include <regex>
#include <memory>
#include <fstream>
#include <unordered_set>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace messaging {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MessagingHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

MessagingHandler::MessagingHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_token{""},
        m_connectionState{ConnectionState::DISCONNECTED},
        m_sendPermission{PermissionState::OFF},
        m_readPermission{PermissionState::OFF} {
    setupUI();
}

std::weak_ptr<Activity> MessagingHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> MessagingHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::messaging::Messaging interface
void MessagingHandler::sendMessage(
    const std::string& token,
    const std::string& message,
    const std::string& recipients) {
    log(logger::LoggerHandler::Level::INFO, "sendMessage:token:" + token);
    m_token = token;

    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }

    auto recipientsJson = json::parse(recipients);
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Send message received");
            console->printLine("Token      : ", token);
            console->printLine("Text       : ", message);
            console->printLine("Recipients :\n", recipientsJson.dump(2));
            console->printRuler();
        }
    });

    // Construct a string version of the send message for display purposes later
    std::string messageInfo = "Message: " + message.substr(0, message.find("\n")) + " Recipients: ";
    for (int i = 0; i < recipientsJson.size(); i++) {
        messageInfo += recipientsJson[i]["address"].get<std::string>();
        if (i < recipientsJson.size() - 1) {
            messageInfo += ", ";
        }
    }

    // Save in list of sent messages
    m_sentMessages.push_back(messageInfo);

    // Send response
    if (m_automaticResponses) {
        if (m_sendPermission == PermissionState::ON) {
            log(logger::LoggerHandler::Level::INFO, "sendMessageSucceeded:token:" + token);
            sendMessageSucceeded(token);
        } else {
            log(logger::LoggerHandler::Level::INFO, "sendMessageFailed:token:" + token);
            sendMessageFailed(token, ErrorCode::NO_PERMISSION, "Send permission is not enabled");
        }
        resetGlobals();
    }
}

void MessagingHandler::uploadConversations(const std::string& token) {
    log(logger::LoggerHandler::Level::INFO, "uploadConversations:token:" + token);
    m_token = token;

    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Upload conversations received");
            console->printLine("Token : ", token);
            console->printRuler();
        }
    });

    if (m_automaticResponses) {
        if (m_conversations != nullptr) {
            // Send conversations report
            conversationsReport(token, m_conversations.dump());
            log(logger::LoggerHandler::Level::INFO, "Conversation report sent for token: " + token);
            resetGlobals();
        } else {
            log(logger::LoggerHandler::Level::ERROR, "Conversation report is not found");
        }
    }
}

void MessagingHandler::updateMessagesStatus(
    const std::string& token,
    const std::string& conversationId,
    const std::string& status) {
    log(logger::LoggerHandler::Level::INFO, "updateMessageStatus:token:" + token);
    m_token = token;

    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            auto statusJson = json::parse(status);

            console->printRuler();
            console->printLine("Update messages status received");
            console->printLine("Token           : ", token);
            console->printLine("Conversation Id : ", conversationId);
            console->printLine("Status          :\n", statusJson.dump(2));
            console->printRuler();

            // Update messages automatically
            if (m_automaticResponses) {
                // Update message status
                updateMessages(conversationId, statusJson);
                // Update engine
                updateMessagesStatusSucceeded(token);
                log(logger::LoggerHandler::Level::INFO, "updateMessageStatusSucceeded:token:" + token);
                resetGlobals();
            } else {
                m_token = token;
                m_status = statusJson;
                m_conversationId = conversationId;
            }
        }
    });
}

void MessagingHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "MessagingHandler", message);
}

// Error code
static const std::map<std::string, MessagingHandler::ErrorCode> ErrorCodeEnumerator{
    {"GENERIC_FAILURE", MessagingHandler::ErrorCode::GENERIC_FAILURE},
    {"NO_CONNECTIVITY", MessagingHandler::ErrorCode::NO_CONNECTIVITY},
    {"NO_PERMISSION", MessagingHandler::ErrorCode::NO_PERMISSION}};

void MessagingHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_applicationContext = activity->getApplicationContext();
    m_console = activity->findViewById("id:console");
    m_automaticResponses = m_applicationContext->isMessagingResponsesEnabled();

    // Handle messaging endpoint connection changes
    activity->registerObserver(Event::onMessagingEndpointConnection, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::INFO, "onMessagingEndpointConnection:" + value);

        static const std::map<std::string, ConnectionState> ConnectionStateEnumerator{
            {"CONNECTED", ConnectionState::CONNECTED}, {"DISCONNECTED", ConnectionState::DISCONNECTED}};

        if (ConnectionStateEnumerator.count(value) == 0) {
            log(logger::LoggerHandler::Level::ERROR, "onMessagingEndpointConnection: Missing or invalid value");
            return false;
        }

        // Update internal connected state
        m_connectionState = ConnectionStateEnumerator.at(value);

        // Notify engine of state change
        updateMessagingEndpointState(m_connectionState, m_sendPermission, m_readPermission);

        // Print message with state value
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Connection state : " + value);
            console->printRuler();
        }

        return true;
    });

    // Handle messaging endpoint permission changes
    activity->registerObserver(Event::onMessagingEndpointPermission, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::INFO, "onMessagingEndpointConnection:" + value);
        static std::regex r("(.+)/(.+)", std::regex::optimize);
        std::smatch sm{};

        // Match permission value
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 2)) {
            return false;
        }

        // Permission type
        static const std::map<std::string, bool> PermissionTypeEnumerator{{"SEND", true}, {"READ", false}};
        // Permission value
        static const std::map<std::string, PermissionState> PermissionStateEnumerator{{"ON", PermissionState::ON},
                                                                                      {"OFF", PermissionState::OFF}};

        // Check that SEND or READ was provided
        if (PermissionTypeEnumerator.count(sm[1]) == 0) {
            log(logger::LoggerHandler::Level::ERROR,
                "onMessagingEndpointPermission: Missing or invalid type: " + sm[1].str());
            return false;
        }

        // Check that ON or OFF was provided
        if (PermissionStateEnumerator.count(sm[2]) == 0) {
            log(logger::LoggerHandler::Level::ERROR,
                "onMessagingEndpointPermission: Missing or invalid value: " + sm[2].str());
            return false;
        }

        // Print message with state value
        if (auto console = m_console.lock()) {
            console->printRuler();

            // Update internal permission state
            if (PermissionTypeEnumerator.at(sm[1])) {
                m_sendPermission = PermissionStateEnumerator.at(sm[2]);
                console->printLine("Send permission : " + sm[2].str());
            } else {
                m_readPermission = PermissionStateEnumerator.at(sm[2]);
                console->printLine("Read permission : " + sm[2].str());
            }

            // Notify engine of state change
            updateMessagingEndpointState(m_connectionState, m_sendPermission, m_readPermission);

            console->printRuler();
        }

        return true;
    });

    // Handle conversations report
    activity->registerObserver(Event::onConversationsReport, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::INFO, "onConversationsReport:" + value);

        // Read the conversations file
        std::ifstream conversationsFile(value);
        if (!conversationsFile.good()) {
            log(logger::LoggerHandler::Level::ERROR, "onConversationsReport: Invalid file path=" + value);
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Invalid file path : " + value);
                console->printRuler();
            }
            // Reset global vars
            resetGlobals();
            return false;
        }

        // Read JSON from file
        conversationsFile >> m_conversations;
        std::stringstream buffer;
        auto valid = validateConversationsReport(buffer);

        if (valid) {
            conversationsReport(m_token, m_conversations.dump());
            // Print message with state value
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Conversation report uploaded : " + value);
                console->printRuler();
            }
            // Reset global vars
            resetGlobals();
            m_readMessages.clear();
            return true;
        } else {
            auto errorMessage = buffer.str() + "\n\nConversations report is invalid due to missing required fields.";
            showMessage(errorMessage, true);
            return false;
        }

        return true;
    });

    // Handle send message result
    activity->registerObserver(Event::onSendMessageStatus, [=](const std::string& value) {
        // Token is required
        if (m_token.empty()) {
            log(logger::LoggerHandler::Level::ERROR, "onSendMessageStatus: token is missing");
            // Print message to console
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Unable to send message response because token is not present");
                console->printRuler();
            }
            return false;
        }

        // Empty value means successful
        if (value.empty()) {
            // Send message succeeded response
            sendMessageSucceeded(m_token);
            log(logger::LoggerHandler::Level::INFO, "onSendMessageStatus:success");
            // Print message to console
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Message succeeded response sent");
                console->printRuler();
            }

            // Reset global vars
            resetGlobals();

            return true;
        }

        // Get error code and message
        static std::regex r("(.+)/(.+)", std::regex::optimize);
        std::smatch sm{};

        // Match permission value
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 2)) {
            log(logger::LoggerHandler::Level::ERROR, "onSendMessageStatus: invalid parameters");
            // Print message to console
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Invalid error code format specified in menu: " + value);
                console->printRuler();
            }
            return false;
        }

        // Check that error message was provided
        if (ErrorCodeEnumerator.count(sm[1]) == 0) {
            log(logger::LoggerHandler::Level::ERROR, "onSendMessageStatus: Invalid error code: " + sm[1].str());
            // Print message to console
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Invalid error code specified in menu: " + sm[1].str());
                console->printRuler();
            }
            return false;
        }

        // Notify engine of send message failure
        sendMessageFailed(m_token, ErrorCodeEnumerator.at(sm[1]), sm[2].str());

        // Print message with state value
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Sent message failed response :\n");
            console->printLine("Code    : " + sm[1].str());
            console->printLine("Message : " + sm[2].str());
            console->printRuler();
            log(logger::LoggerHandler::Level::INFO,
                "onSendMessageStatus: token=" + m_token + " code=" + sm[1].str() + " message=" + sm[2].str());
        }

        // Clear token
        m_token = "";

        return true;
    });

    // Handle update messages status result
    activity->registerObserver(Event::onUpdateMessagesStatus, [=](const std::string& value) {
        // Token is required
        if (m_token.empty()) {
            log(logger::LoggerHandler::Level::ERROR, "onUpdateMessagesStatus: token is missing");
            // Print message to console
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Unable to send update messages status because token is not present");
                console->printRuler();
            }
            return false;
        }

        // Empty value means successful
        if (value.empty()) {
            if (m_status != nullptr) {
                // Update messages
                updateMessages(m_conversationId, m_status);
                // Send update messages status succeeded response
                updateMessagesStatusSucceeded(m_token);
                log(logger::LoggerHandler::Level::INFO, "onUpdateMessagesStatus:success");
                // Print message to console
                if (auto console = m_console.lock()) {
                    console->printRuler();
                    console->printLine("Update messages status response sent");
                    console->printRuler();
                }

                // Reset global vars
                resetGlobals();

                return true;
            } else {
                log(logger::LoggerHandler::Level::ERROR,
                    "Cannot send message failed response due to empty conversation id and status");
                return false;
            }
        }

        // Get error code and message
        static std::regex r("(.+)/(.+)", std::regex::optimize);
        std::smatch sm{};

        // Match permission value
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 2)) {
            log(logger::LoggerHandler::Level::ERROR, "onUpdateMessagesStatus: invalid parameters");
            // Print message to console
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Invalid error code format specified in menu: " + value);
                console->printRuler();
            }
            return false;
        }

        // Check that error message was provided
        if (ErrorCodeEnumerator.count(sm[1]) == 0) {
            log(logger::LoggerHandler::Level::ERROR, "onUpdateMessagesStatus: Invalid error code: " + sm[1].str());
            // Print message to console
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Invalid error code specified in menu: " + sm[1].str());
                console->printRuler();
            }
            return false;
        }

        // Notify engine of update messages status failure
        updateMessagesStatusFailed(m_token, ErrorCodeEnumerator.at(sm[1]), sm[2].str());

        // Print message with state value
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Sent update message status response :\n");
            console->printLine("Code    : " + sm[1].str());
            console->printLine("Message : " + sm[2].str());
            console->printRuler();
            log(logger::LoggerHandler::Level::INFO,
                "onUpdateMessagesStatus: token=" + m_token + " code=" + sm[1].str() + " message=" + sm[2].str());
        }

        // Reset global vars
        resetGlobals();

        return true;
    });

    // Handle toggle automatic responses
    activity->registerObserver(Event::onToggleAutomaticResponses, [=](const std::string& value) {
        m_automaticResponses = m_applicationContext->isMessagingResponsesEnabled();

        // Toggle
        m_automaticResponses = !m_automaticResponses;
        m_applicationContext->setMessagingResponses(m_automaticResponses);
        std::string responses = m_automaticResponses ? "ON" : "OFF";

        // Print message
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Automatic responses : " + responses);
            console->printRuler();
        }

        log(logger::LoggerHandler::Level::VERBOSE, "onToggleAutomaticResponses:" + responses);
        return true;
    });

    // Handle show messaging info
    activity->registerObserver(Event::onShowMessagingInfo, [=](const std::string& value) {
        std::stringstream buffer;

        std::string connected = m_connectionState == ConnectionState::CONNECTED ? "CONNECTED" : "DISCONNECTED";
        std::string sendPermission = m_sendPermission == PermissionState::ON ? "ON" : "OFF";
        std::string readPermission = m_readPermission == PermissionState::ON ? "ON" : "OFF";
        std::string automaticResponses = m_automaticResponses ? "ON" : "OFF";

        buffer << "Messaging information" << std::endl
               << std::endl
               << "Connection state    : " << connected << std::endl
               << "Send permission     : " << sendPermission << std::endl
               << "Read permission     : " << readPermission << std::endl
               << "Automatic responses : " << automaticResponses << std::endl;

        if (!m_automaticResponses) {
            buffer << "Token               : " << m_token << std::endl;
        }

        auto valid = validateConversationsReport(buffer);

        if (!m_readMessages.empty()) {
            buffer << "\nRead messages\n" << std::endl;
            for (int i = 0; i < m_readMessages.size(); i++) {
                buffer << "  " << m_readMessages[i] << std::endl;
            }
        }

        if (!m_sentMessages.empty()) {
            buffer << "\nSent messages\n" << std::endl;
            for (int i = 0; i < m_sentMessages.size(); i++) {
                buffer << "  " << m_sentMessages[i] << std::endl;
            }
        }

        // Conversations report is not valid
        if (!valid) {
            buffer << "\nERROR: Conversations report is invalid due to missing required fields";
        }

        showMessage(buffer.str());
        return true;
    });
}

bool MessagingHandler::validateConversationsReport(std::stringstream& buffer) {
    bool isValid = true;

    // Conversations array
    if (m_conversations.is_array()) {
        if (!m_conversations.empty()) {
            buffer << "\nConversations Report" << std::endl;
            for (auto conversation : m_conversations) {
                if (conversation.find("id") != conversation.end()) {
                    buffer << "\n  Conversation id : " << conversation["id"].get<std::string>() << std::endl;
                } else {
                    buffer << "\n  Conversation id : Not found\n";
                    isValid = false;
                }

                // Print other participants
                if (conversation.find("otherParticipants") != conversation.end()) {
                    auto participants = conversation["otherParticipants"];
                    if (!participants.empty()) {
                        buffer << "\n    Participants" << std::endl << std::endl;
                        // Print participants
                        for (auto participant : participants) {
                            if (participant.find("address") != participant.end()) {
                                buffer << "      " << participant["address"].get<std::string>() << std::endl;
                            } else {
                                buffer << "      Phone number is missing" << std::endl;
                                isValid = false;
                            }

                            if (participant.find("addressType") == participant.end()) {
                                buffer << "      addressType : Not Found" << std::endl;
                                isValid = false;
                            } else {
                                if (participant["addressType"] != "PhoneNumberAddress") {
                                    buffer << "       addressType: Expected 'PhoneNumberAddress' Found '"
                                           << participant["addressType"] << "'" << std::endl;
                                    isValid = false;
                                }
                            }
                        }
                        buffer << std::endl;
                    }
                } else {
                    buffer << "\n    Participants : Not found" << std::endl << std::endl;
                    isValid = false;
                }

                // Print messages
                if (conversation.find("messages") != conversation.end()) {
                    auto messages = conversation["messages"];
                    if (!messages.empty()) {
                        buffer << "    Messages" << std::endl;
                        // Print messages
                        for (int i = 0; i < messages.size(); i++) {
                            auto message = messages[i];

                            buffer << "\n      Message id  : ";
                            if (message.find("id") != message.end()) {
                                buffer << message["id"].get<std::string>();
                            } else {
                                buffer << "Not found";
                                isValid = false;
                            }

                            buffer << "\n      Text        : ";
                            if (message.find("messagePayload") != message.end()) {
                                auto payload = message["messagePayload"];
                                if (payload.find("text") != payload.end()) {
                                    buffer << payload["text"].get<std::string>();
                                } else {
                                    buffer << "Not found";
                                    isValid = false;
                                }

                                if (payload.find("@type") == payload.end()) {
                                    buffer << "\n      @type       : Not found";
                                    isValid = false;
                                } else {
                                    if (payload["@type"].get<std::string>() != "text") {
                                        buffer << "\n      @type       : Expected 'text' found '"
                                               << payload["@type"].get<std::string>() << "'";
                                        isValid = false;
                                    }
                                }
                            } else {
                                buffer << "Not found";
                                isValid = false;
                            }

                            buffer << "\n      Status      : ";
                            if (message.find("status") != message.end()) {
                                buffer << message["status"].get<std::string>();
                            } else {
                                buffer << "Not found";
                                isValid = false;
                            }

                            buffer << "\n      Time        : ";
                            if (message.find("createdTime") != message.end()) {
                                buffer << message["createdTime"].get<std::string>();
                            } else {
                                buffer << "Not found";
                                isValid = false;
                            }

                            buffer << "\n      Sender      : ";
                            if (message.find("sender") != message.end()) {
                                auto sender = message["sender"];
                                if (sender.find("address") != sender.end()) {
                                    buffer << sender["address"].get<std::string>();
                                } else {
                                    buffer << "Not found";
                                    isValid = false;
                                }
                                if (sender.find("addressType") == sender.end()) {
                                    buffer << "\n      addressType : Not found";
                                    isValid = false;
                                } else {
                                    if (sender["addressType"] != "PhoneNumberAddress") {
                                        buffer << "\n      addressType : Expected 'PhoneNumberAddress' found '"
                                               << sender["addressType"] << "'";
                                        isValid = false;
                                    }
                                }
                            } else {
                                buffer << "Not found";
                                isValid = false;
                            }

                            buffer << std::endl;
                            // Space out messages
                            if (i < messages.size() - 1) {
                                buffer << std::endl;
                            }
                        }
                    }
                } else {
                    buffer << "    Messages : Not found" << std::endl << std::endl;
                    isValid = false;
                }
            }
        }

    } else {
        buffer << "\nConversations report is not a JSON array.";
        isValid = false;
    }

    return isValid;
}

void MessagingHandler::updateMessages(const std::string& conversationId, const json& statusJson) {
    // Build map of read messages to delete
    std::unordered_set<std::string> readSet;
    for (auto id : statusJson["read"]) {
        readSet.insert(id.get<std::string>());
    }

    // Update conversations array
    for (int i = 0; i < m_conversations.size(); i++) {
        if (conversationId == m_conversations[i]["id"]) {
            auto messages = m_conversations[i]["messages"];
            std::vector<int> messagesToErase;
            for (int j = 0; j < messages.size(); j++) {
                if (readSet.find(messages[j]["id"]) != readSet.end()) {
                    auto message = messages[j];
                    m_readMessages.push_back(
                        "Conversation Id: " + conversationId + " Message Id: " + message["id"].get<std::string>() +
                        " Text: " + message["messagePayload"]["text"].get<std::string>());
                    messagesToErase.push_back(j);
                }
            }

            // Sort from biggest index to remove from messages array
            std::sort(messagesToErase.rbegin(), messagesToErase.rend());
            for (int& it : messagesToErase) {
                m_conversations[i]["messages"].erase(m_conversations[i]["messages"].begin() + it);
            }

            break;
        }
    }
}

void MessagingHandler::showMessage(const std::string& message, bool asError) {
    // Log as error or info
    if (asError) {
        log(logger::LoggerHandler::Level::ERROR, message);
    } else {
        log(logger::LoggerHandler::Level::INFO, message);
    }

    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine(message);
        console->printRuler();
    }
}

void MessagingHandler::resetGlobals() {
    m_token = "";
    m_conversationId = "";
    m_status = nullptr;
}

}  // namespace messaging
}  // namespace sampleApp
