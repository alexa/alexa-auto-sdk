/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Alexa/AlexaClientHandler.h"

#include <AASB/Message/Alexa/AlexaClient/StopForegroundActivityMessage.h>

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::alexaClient;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaClientHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AlexaClientHandler::AlexaClientHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr) && (m_messageBroker != nullptr));
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> AlexaClientHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AlexaClientHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void AlexaClientHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to DialogStateChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleDialogStateChangedMessage(message); },
        DialogStateChangedMessage::topic(),
        DialogStateChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to AuthStateChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAuthStateChangedMessage(message); },
        AuthStateChangedMessage::topic(),
        AuthStateChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to ConnectionStatusChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleConnectionStatusChangedMessage(message); },
        ConnectionStatusChangedMessage::topic(),
        ConnectionStatusChangedMessage::action());
}

void AlexaClientHandler::handleDialogStateChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received DialogStateChangedMessage");
    DialogStateChangedMessage msg = json::parse(message);
    dialogStateChanged(msg.payload.state);
}

void AlexaClientHandler::handleAuthStateChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AuthStateChangedMessage");
    AuthStateChangedMessage msg = json::parse(message);
    authStateChanged(msg.payload.state, msg.payload.error);
}

void AlexaClientHandler::handleConnectionStatusChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received ConnectionStatusChangedMessage");
    ConnectionStatusChangedMessage msg = json::parse(message);
    connectionStatusChanged(msg.payload.status, msg.payload.reason);
}

void AlexaClientHandler::dialogStateChanged(DialogState state) {
    std::stringstream ss;
    ss << convertDialogStateToString(state);
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    auto text = ss.str();
    activity->runOnUIThread([=]() {
        if (auto dialogStateView = m_dialogStateView.lock()) {
            dialogStateView->setText(text);
        }
        if (auto console = m_console.lock()) {
            console->printLine("Dialog state changed:", convertDialogStateToString(state));
        }
    });
    // Special case for test automation
    if (state == DialogState::IDLE) {
        if (auto executor = activity->getExecutor()) {
            executor->submit([=]() { activity->notify(Event::onTestAutomationProcess); });
        }
    }
}

void AlexaClientHandler::authStateChanged(AuthState state, AuthError error) {
    std::stringstream ss;
    ss << convertAuthStateToString(state) << '/' << convertAuthErrorToString(error);
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    auto text = ss.str();
    activity->runOnUIThread([=]() {
        if (auto authStateView = m_authStateView.lock()) {
            authStateView->setText(text);
        }
        if (auto console = m_console.lock()) {
            console->printLine(
                "Auth state changed:", convertAuthStateToString(state), "(", convertAuthErrorToString(error), ")");
        }
    });
}

void AlexaClientHandler::connectionStatusChanged(ConnectionStatus status, ConnectionChangedReason reason) {
    std::stringstream ss;
    ss << convertConnectionStatusToString(status) << '/' << convertConnectionChangedReasonToString(reason);
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    auto text = ss.str();
    activity->runOnUIThread([=]() {
        if (auto connectionStatusView = m_connectionStatusView.lock()) {
            connectionStatusView->setText(text);
        }
        if (auto console = m_console.lock()) {
            console->printLine(
                "Connection status changed:",
                convertConnectionStatusToString(status),
                "(",
                convertConnectionChangedReasonToString(reason),
                ")");
        }
    });
    // Special case for test automation
    if (status == ConnectionStatus::CONNECTED) {
        if (reason == ConnectionChangedReason::ACL_CLIENT_REQUEST) {
            if (auto executor = activity->getExecutor()) {
                executor->submit([=]() { activity->notify(Event::onTestAutomationConnect); });
            }
        }
    }
}

void AlexaClientHandler::stopForegroundActivity() {
    // Publish the "StopForegroundActivity" message
    StopForegroundActivityMessage msg;
    m_messageBroker->publish(msg.toString());
}

void AlexaClientHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AlexaClientHandler", message);
}

void AlexaClientHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // text views
    m_authStateView = activity->findViewById("id:AuthState");
    m_connectionStatusView = activity->findViewById("id:ConnectionStatus");
    m_dialogStateView = activity->findViewById("id:DialogState");

    activity->registerObserver(Event::onStopForegroundActivity, [=](const std::string&) {
        log(logger::LoggerHandler::Level::INFO, "onStopForegroundActivity");
        stopForegroundActivity();
        return true;
    });

    // initial text views
    activity->runOnUIThread([=]() {
        if (auto authStateView = m_authStateView.lock()) {
            std::stringstream ss;
            ss << convertAuthStateToString(AuthState::UNINITIALIZED);
            authStateView->setText(ss.str());
        }
        if (auto connectionStatusView = m_connectionStatusView.lock()) {
            std::stringstream ss;
            ss << convertConnectionStatusToString(ConnectionStatus::DISCONNECTED);
            connectionStatusView->setText(ss.str());
        }
        if (auto dialogStateView = m_dialogStateView.lock()) {
            std::stringstream ss;
            ss << convertDialogStateToString(DialogState::IDLE);
            dialogStateView->setText(ss.str());
        }
    });
}

std::string AlexaClientHandler::convertDialogStateToString(const DialogState& state) {
    std::string dialogStateStr;
    switch (state) {
        case DialogState::IDLE:
            return "IDLE";
        case DialogState::LISTENING:
            return "LISTENING";
        case DialogState::EXPECTING:
            return "EXPECTING";
        case DialogState::THINKING:
            return "THINKING";
        case DialogState::SPEAKING:
            return "SPEAKING";
    }
    // Invalid dialog state
    log(logger::LoggerHandler::Level::ERROR, "Invalid Dialog State");
    return "";
}

std::string AlexaClientHandler::convertAuthStateToString(const AuthState& state) {
    switch (state) {
        case AuthState::EXPIRED:
            return "EXPIRED";
        case AuthState::REFRESHED:
            return "REFRESHED";
        case AuthState::UNINITIALIZED:
            return "UNINITIALIZED";
        case AuthState::UNRECOVERABLE_ERROR:
            return "UNRECOVERABLE_ERROR";
    }
    // Invalid auth state
    log(logger::LoggerHandler::Level::ERROR, "Invalid Auth State.");
    return "";
}

std::string AlexaClientHandler::convertAuthErrorToString(const AuthError& error) {
    switch (error) {
        case AuthError::NO_ERROR:
            return "NO_ERROR";
            break;
        case AuthError::UNKNOWN_ERROR:
            return "UNKNOWN_ERROR";
            break;
        case AuthError::AUTHORIZATION_FAILED:
            return "AUTHORIZATION_FAILED";
            break;
        case AuthError::UNAUTHORIZED_CLIENT:
            return "UNAUTHORIZED_CLIENT";
            break;
        case AuthError::SERVER_ERROR:
            return "SERVER_ERROR";
            break;
        case AuthError::INVALID_REQUEST:
            return "INVALID_REQUEST";
            break;
        case AuthError::AUTHORIZATION_EXPIRED:
            return "AUTHORIZATION_EXPIRED";
            break;
        case AuthError::UNSUPPORTED_GRANT_TYPE:
            return "UNSUPPORTED_GRANT_TYPE";
            break;
        case AuthError::AUTHORIZATION_PENDING:
            return "AUTHORIZATION_PENDING";
            break;
        case AuthError::INTERNAL_ERROR:
            return "INTERNAL_ERROR";
            break;
        case AuthError::INVALID_CBL_CLIENT_ID:
            return "INVALID_CBL_CLIENT_ID";
            break;
        case AuthError::INVALID_CODE_PAIR:
            return "INVALID_CODE_PAIR";
            break;
        case AuthError::INVALID_VALUE:
            return "INVALID_VALUE";
            break;
        case AuthError::SLOW_DOWN:
            return "SLOW_DOWN";
            break;
    }
    // Invalid auth error
    log(logger::LoggerHandler::Level::ERROR, "Invalid Auth Error.");
    return "";
}

std::string AlexaClientHandler::convertConnectionStatusToString(const ConnectionStatus& status) {
    switch (status) {
        case ConnectionStatus::DISCONNECTED:
            return "DISCONNECTED";
        case ConnectionStatus::PENDING:
            return "PENDING";
        case ConnectionStatus::CONNECTED:
            return "CONNECTED";
    }
    // Invalid connection status
    log(logger::LoggerHandler::Level::ERROR, "Invalid Connection Status.");
    return "";
}

std::string AlexaClientHandler::convertConnectionChangedReasonToString(const ConnectionChangedReason& reason) {
    switch (reason) {
        case ConnectionChangedReason::NONE:
            return "NONE";
        case ConnectionChangedReason::SUCCESS:
            return "SUCCESS";
        case ConnectionChangedReason::UNRECOVERABLE_ERROR:
            return "UNRECOVERABLE_ERROR";
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
            return "SERVER_ENDPOINT_CHANGED";
    }
    // Invalid connection changed reason
    log(logger::LoggerHandler::Level::ERROR, "Invalid Connection Changed Reason.");
    return "";
}

}  // namespace alexa
}  // namespace sampleApp
