/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "AlexaClientHandler.h"

#include <aasb/Consts.h>
#include "DirectiveDispatcher.h"
#include "PlatformSpecificLoggingMacros.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace alexa {

const std::string TAG = "aasb::alexa::AlexaClientHandler";

std::shared_ptr<AlexaClientHandler> AlexaClientHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) {
    auto alexaClientHandler = std::shared_ptr<AlexaClientHandler>(new AlexaClientHandler(directiveDispatcher));
    alexaClientHandler->m_logger = logger;
    return alexaClientHandler;
}

AlexaClientHandler::AlexaClientHandler(std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) :
        m_directiveDispatcher(directiveDispatcher) {
}

void AlexaClientHandler::dialogStateChanged(DialogState state) {
    m_dialogState = state;
    std::string info = std::string(": DialogState: ") + std::string(convertDialogStateToString(state));
    m_logger->log(Level::INFO, TAG, info);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(
            aasb::bridge::TOPIC_ALEXA_CLIENT,
            aasb::bridge::ACTION_DIALOG_STATE_CHANGED,
            convertDialogStateToString(state));
    }
}

void AlexaClientHandler::connectionStatusChanged(ConnectionStatus status, ConnectionChangedReason reason) {
    m_connectionStatus = status;
    std::string info = std::string(": ConnectionStatus: ") + std::string(convertConnectionStatusToString(status)) +
                       std::string(": ConnectionChangedReason: ") +
                       std::string(convertConnectionChangedReasonToString(reason));

    m_logger->log(Level::INFO, TAG, info);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(
            aasb::bridge::TOPIC_ALEXA_CLIENT,
            aasb::bridge::ACTION_CONNECTION_STATUS_CHANGED,
            convertConnectionStatusToString(status));
    }
}

void AlexaClientHandler::authStateChanged(AuthState state, AuthError error) {
    m_authState = state;
    std::string info = std::string(": AuthState: ") + std::string(convertAuthStateToString(state)) +
                       std::string(": AuthError: ") + std::string(convertAuthErrorToString(error));
    m_logger->log(Level::INFO, TAG, info);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(
            aasb::bridge::TOPIC_ALEXA_CLIENT, aasb::bridge::ACTION_AUTH_STATE_CHANGED, convertAuthStateToString(state));
    }
}

std::string AlexaClientHandler::getConnectionStatus() const {
    return convertConnectionStatusToString(m_connectionStatus);
}

std::string AlexaClientHandler::getAuthState() const {
    return convertAuthStateToString(m_authState);
}

std::string AlexaClientHandler::getDialogState() const {
    return convertDialogStateToString(m_dialogState);
}

std::string AlexaClientHandler::convertConnectionStatusToString(ConnectionStatus status) {
    switch (status) {
        case ConnectionStatus::CONNECTED:
            return "CONNECTED";
        case ConnectionStatus::DISCONNECTED:
            return "DISCONNECTED";
        case ConnectionStatus::PENDING:
            return "PENDING";
        default:
            return std::string("UNKNOWN");
    }
}

std::string AlexaClientHandler::convertDialogStateToString(DialogState state) {
    switch (state) {
        case DialogState::LISTENING:
            return "LISTENING";
        case DialogState::SPEAKING:
            return "SPEAKING";
        case DialogState::THINKING:
            return "THINKING";
        case DialogState::IDLE:
            return "IDLE";
        default:
            return std::string("UNKNOWN");
    }
}

std::string AlexaClientHandler::convertConnectionChangedReasonToString(ConnectionChangedReason reason) {
    switch (reason) {
        case ConnectionChangedReason::ACL_CLIENT_REQUEST:
            return "ACL_CLIENT_REQUEST";
        case ConnectionChangedReason::ACL_DISABLED:
            return "ACL_DISABLED";
        case ConnectionChangedReason::DNS_TIMEDOUT:
            return "DNS_TIMEDOUT";
        case ConnectionChangedReason::CONNECTION_TIMEDOUT:
            return "CONNECTION_TIMEDOUT";
        case ConnectionChangedReason::CONNECTION_THROTTLED:
            return "CONNECTION_THROTTLED";
        case ConnectionChangedReason::INVALID_AUTH:
            return "INVALID_AUTH";
        case ConnectionChangedReason::PING_TIMEDOUT:
            return "PING_TIMEDOUT";
        case ConnectionChangedReason::WRITE_TIMEDOUT:
            return "WRITE_TIMEDOUT";
        case ConnectionChangedReason::READ_TIMEDOUT:
            return "READ_TIMEDOUT";
        case ConnectionChangedReason::FAILURE_PROTOCOL_ERROR:
            return "FAILURE_PROTOCOL_ERROR";
        case ConnectionChangedReason::INTERNAL_ERROR:
            return "INTERNAL_ERROR";
        case ConnectionChangedReason::SERVER_INTERNAL_ERROR:
            return "SERVER_INTERNAL_ERROR";
        case ConnectionChangedReason::SERVER_SIDE_DISCONNECT:
            return "SERVER_SIDE_DISCONNECT";
        case ConnectionChangedReason::SERVER_ENDPOINT_CHANGED:
            return "INVALID_AUTH";
        default:
            return std::string("UNKNOWN");
    }
}

std::string AlexaClientHandler::convertAuthStateToString(AuthState state) {
    switch (state) {
        case AuthState::UNINITIALIZED:
            return "UNINITIALIZED";
        case AuthState::REFRESHED:
            return "REFRESHED";
        case AuthState::EXPIRED:
            return "EXPIRED";
        case AuthState::UNRECOVERABLE_ERROR:
            return "UNRECOVERABLE_ERROR";
        default:
            return std::string("UNKNOWN");
    }
}

std::string AlexaClientHandler::convertAuthErrorToString(AuthError error) {
    switch (error) {
        case AuthError::NO_ERROR:
            return "NO_ERROR";
        case AuthError::UNKNOWN_ERROR:
            return "UNKNOWN_ERROR";
        case AuthError::AUTHORIZATION_FAILED:
            return "AUTHORIZATION_FAILED";
        case AuthError::UNAUTHORIZED_CLIENT:
            return "UNAUTHORIZED_CLIENT";
        case AuthError::SERVER_ERROR:
            return "SERVER_ERROR";
        case AuthError::INVALID_REQUEST:
            return "INVALID_REQUEST";
        case AuthError::AUTHORIZATION_EXPIRED:
            return "AUTHORIZATION_EXPIRED";
        case AuthError::UNSUPPORTED_GRANT_TYPE:
            return "UNSUPPORTED_GRANT_TYPE";
        default:
            return std::string("UNKNOWN");
    }
}

}  // namespace alexa
}  // namespace aasb