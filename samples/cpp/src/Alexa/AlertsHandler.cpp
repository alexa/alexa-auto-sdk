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

#include "SampleApp/Alexa/AlertsHandler.h"

#include <AASB/Message/Alexa/Alerts/AlertCreatedMessage.h>
#include <AASB/Message/Alexa/Alerts/AlertDeletedMessage.h>
#include <AASB/Message/Alexa/Alerts/AlertStateChangedMessage.h>
#include <AASB/Message/Alexa/Alerts/LocalStopMessage.h>
#include <AASB/Message/Alexa/Alerts/RemoveAllAlertsMessage.h>

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::alerts;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlertsHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// aliases
using MessageBroker = aace::core::MessageBroker;

AlertsHandler::AlertsHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr) && (m_messageBroker != nullptr));
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> AlertsHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AlertsHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void AlertsHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to AlertStateChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAlertStateChangedMessage(message); },
        AlertStateChangedMessage::topic(),
        AlertStateChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to AlertCreatedMessage Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAlertCreatedMessage(message); },
        AlertCreatedMessage::topic(),
        AlertCreatedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to AlertDeletedMessage Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAlertDeletedMessage(message); },
        AlertDeletedMessage::topic(),
        AlertDeletedMessage::action());
}

void AlertsHandler::handleAlertStateChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AlertStateChangedMessage");
    AlertStateChangedMessage msg = json::parse(message);
    alertStateChanged(msg.payload.alertToken, msg.payload.state, msg.payload.reason);
}

void AlertsHandler::handleAlertCreatedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AlertCreatedMessage");
    AlertCreatedMessage msg = json::parse(message);
    alertCreated(msg.payload.alertToken, msg.payload.detailedInfo);
}

void AlertsHandler::handleAlertDeletedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AlertDeletedMessage");
    AlertDeletedMessage msg = json::parse(message);
    alertDeleted(msg.payload.alertToken);
}

void AlertsHandler::localStop() {
    // Publish the "LocalStop" message
    LocalStopMessage msg;
    m_messageBroker->publish(msg.toString());
}

void AlertsHandler::removeAllAlerts() {
    // Publish the "RemoveAllAlerts" message
    RemoveAllAlertsMessage msg;
    m_messageBroker->publish(msg.toString());
}

void AlertsHandler::alertStateChanged(const std::string& alertToken, AlertState state, const std::string& reason) {
    std::stringstream ss;
    switch (state) {
        case AlertState::READY:
            ss << "READY";
            break;
        case AlertState::STARTED:
            ss << "STARTED";
            break;
        case AlertState::STOPPED:
            ss << "STOPPED";
            break;
        case AlertState::SNOOZED:
            ss << "SNOOZED";
            break;
        case AlertState::COMPLETED:
            ss << "COMPLETED";
            break;
        case AlertState::PAST_DUE:
            ss << "PAST_DUE";
            break;
        case AlertState::FOCUS_ENTERED_FOREGROUND:
            ss << "FOCUS_ENTERED_FOREGROUND";
            break;
        case AlertState::FOCUS_ENTERED_BACKGROUND:
            ss << "FOCUS_ENTERED_BACKGROUND";
            break;
        case AlertState::ERROR:
            ss << "ERROR";
            break;
        case AlertState::DELETED:
            ss << "DELETED";
            break;
        case AlertState::SCHEDULED_FOR_LATER:
            ss << "SCHEDULED_FOR_LATER";
            break;
    }

    log(logger::LoggerHandler::Level::INFO, "alertStateChanged:state=" + ss.str() + ",reason=" + reason);
    if (!reason.empty()) {
        ss << '/' << reason;
    }
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    auto text = ss.str();
    activity->runOnUIThread([=]() {
        if (auto alertStateView = m_alertStateView.lock()) {
            alertStateView->setText(text);
        }
        if (auto console = m_console.lock()) {
            if (!reason.empty()) {
                console->printLine("Alert state changed:", text, "(", reason, ")");
            } else {
                console->printLine("Alert state changed:", text);
            }
        }
    });
    return;
}

void AlertsHandler::alertCreated(const std::string& alertToken, const std::string& detailedInfo) {
    log(logger::LoggerHandler::Level::INFO, "alertCreated:alertToken=" + alertToken + ",detailedInfo=" + detailedInfo);
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("Alert created:", alertToken);
        }
    });
}

void AlertsHandler::alertDeleted(const std::string& alertToken) {
    log(logger::LoggerHandler::Level::INFO, "alertDeleted:alertToken=" + alertToken);
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("Alert deleted:", alertToken);
        }
    });
}

void AlertsHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AlertsHandler", message);
}

void AlertsHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // text views
    m_alertStateView = activity->findViewById("id:AlertState");

    // initial view text
    activity->runOnUIThread([=]() {
        if (auto alertStateView = m_alertStateView.lock()) {
            alertStateView->setText("");
        }
    });

    // exit the active domain
    activity->registerObserver(Event::onStopActive, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onStopActive");
        localStop();
        return true;
    });

    // localStop
    activity->registerObserver(Event::onAlertsLocalStop, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onAlertsLocalStop");
        localStop();
        return true;
    });

    // removeAllAlerts
    activity->registerObserver(Event::onAlertsRemoveAllAlerts, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onAlertsRemoveAllAlerts");
        removeAllAlerts();
        return true;
    });
}

}  // namespace alexa
}  // namespace sampleApp
