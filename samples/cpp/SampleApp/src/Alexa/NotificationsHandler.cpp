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

#include "SampleApp/Alexa/NotificationsHandler.h"

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NotificationsHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

NotificationsHandler::NotificationsHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    // Expects((mediaPlayer != nullptr) && (speaker != nullptr));
    setupUI();
}

std::weak_ptr<Activity> NotificationsHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> NotificationsHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::Notifications interface

void NotificationsHandler::setIndicator(Notifications::IndicatorState state) {
    std::stringstream ss;
    ss << state;
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
            console->printLine("Set indicator:", state);
        }
    });
}

void NotificationsHandler::onNotificationReceived() {
    std::stringstream ss;
    ss << "onNotificationsReceived";
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

// private

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
