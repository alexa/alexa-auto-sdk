/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaClientHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AlexaClientHandler::AlexaClientHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> AlexaClientHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AlexaClientHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::AlexaClient interface

void AlexaClientHandler::dialogStateChanged(AlexaClient::DialogState state) {
    std::stringstream ss;
    ss << state;
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
            console->printLine("Dialog state changed:", state);
        }
    });
    // Special case for test automation
    if (state == AlexaClient::DialogState::IDLE) {
        if (auto executor = activity->getExecutor()) {
            executor->submit([=]() { activity->notify(Event::onTestAutomationProcess); });
        }
    }
}

void AlexaClientHandler::authStateChanged(AlexaClient::AuthState state, AlexaClient::AuthError error) {
    std::stringstream ss;
    ss << state << '/' << error;
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
            console->printLine("Auth state changed:", state, "(", error, ")");
        }
    });
}

void AlexaClientHandler::connectionStatusChanged(
    AlexaClient::ConnectionStatus status,
    AlexaClient::ConnectionChangedReason reason) {
    std::stringstream ss;
    ss << status << '/' << reason;
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
            console->printLine("Connection status changed:", status, "(", reason, ")");
        }
    });
    // Special case for test automation
    if (status == AlexaClient::ConnectionStatus::CONNECTED) {
        if (reason == AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST) {
            if (auto executor = activity->getExecutor()) {
                executor->submit([=]() { activity->notify(Event::onTestAutomationConnect); });
            }
        }
    }
}

// private

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
            ss << AlexaClient::AuthState::UNINITIALIZED;
            authStateView->setText(ss.str());
        }
        if (auto connectionStatusView = m_connectionStatusView.lock()) {
            std::stringstream ss;
            ss << AlexaClient::ConnectionStatus::DISCONNECTED;
            connectionStatusView->setText(ss.str());
        }
        if (auto dialogStateView = m_dialogStateView.lock()) {
            std::stringstream ss;
            ss << AlexaClient::DialogState::IDLE;
            dialogStateView->setText(ss.str());
        }
    });
}

}  // namespace alexa
}  // namespace sampleApp
