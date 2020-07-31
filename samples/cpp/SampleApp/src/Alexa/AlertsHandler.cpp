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

#include "SampleApp/Alexa/AlertsHandler.h"

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlertsHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AlertsHandler::AlertsHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    // Expects((mediaPlayer != nullptr) && (speaker != nullptr));
    setupUI();
}

std::weak_ptr<Activity> AlertsHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AlertsHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::Alerts interface

void AlertsHandler::alertStateChanged(
    const std::string& alertToken,
    Alerts::AlertState state,
    const std::string& reason) {
    std::stringstream ss;
    ss << state;
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
                console->printLine("Alert state changed:", state, "(", reason, ")");
            } else {
                console->printLine("Alert state changed:", state);
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

// private

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
