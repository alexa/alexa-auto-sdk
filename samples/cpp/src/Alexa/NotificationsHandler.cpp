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

#include "SampleApp/Alexa/NotificationsHandler.h"

#include <AASB/Message/Alexa/Notifications/SetIndicatorMessage.h>
#include <AASB/Message/Alexa/Notifications/OnNotificationReceivedMessage.h>

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::notifications;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NotificationsHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// aliases
using MessageBroker = aace::core::MessageBroker;

NotificationsHandler::NotificationsHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr) && (m_messageBroker != nullptr)););
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> NotificationsHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> NotificationsHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void NotificationsHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to OnNotificationReceived Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleOnNotificationReceived(message); },
        OnNotificationReceivedMessage::topic(),
        OnNotificationReceivedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to SetIndicator Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSetIndicatorMessage(message); },
        SetIndicatorMessage::topic(),
        SetIndicatorMessage::action());
}

void NotificationsHandler::handleSetIndicatorMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received SetIndicatorMessage");
    SetIndicatorMessage msg = json::parse(message);
    setIndicator(msg.payload.state);
}

void NotificationsHandler::setIndicator(IndicatorState state) {
    std::stringstream ss;
    switch (state) {
        case IndicatorState::OFF:
            ss << "OFF";
            break;
        case IndicatorState::ON:
            ss << "ON";
            break;
        case IndicatorState::UNKNOWN:
            ss << "UNKNOWN";
            break;
    }
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    auto text = ss.str();
    activity->runOnUIThread([=]() {
        if (auto indicatorStateView = m_indicatorStateView.lock()) {
            indicatorStateView->setText(text);
        }
        if (auto console = m_console.lock()) {
            console->printLine("Set indicator:", text);
        }
    });
}

void NotificationsHandler::handleOnNotificationReceived(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received OnNotificationReceived");
}

void NotificationsHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "NotificationsHandler", message);
}

void NotificationsHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // text views
    m_indicatorStateView = activity->findViewById("id:IndicatorState");

    // initial text views
    activity->runOnUIThread([=]() {
        if (auto indicatorStateView = m_indicatorStateView.lock()) {
            indicatorStateView->setText("");
        }
    });
}

}  // namespace alexa
}  // namespace sampleApp
