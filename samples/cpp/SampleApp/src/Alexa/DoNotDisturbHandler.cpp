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

#include "SampleApp/Alexa/DoNotDisturbHandler.h"

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

DoNotDisturbHandler::DoNotDisturbHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    // Expects((mediaPlayer != nullptr) && (speaker != nullptr));
    setupUI();
}

std::weak_ptr<Activity> DoNotDisturbHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> DoNotDisturbHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::DoNotDisturb interface

void DoNotDisturbHandler::setDoNotDisturb(const bool doNotDisturb) {
    std::stringstream ss;
    ss << doNotDisturb;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (activity) {
        auto text = ss.str();
        activity->runOnUIThread([=]() {
            if (auto doNotDisturbStateView = m_doNotDisturbStateView.lock()) {
                doNotDisturbStateView->setText(text);
            }
            if (auto console = m_console.lock()) {
                console->printRuler();
                std::string boolean = doNotDisturb ? "true" : "false";
                console->printLine("DoNotDisturb Set: " + boolean);
                console->printRuler();
            }
        });
    }
}

// private

void DoNotDisturbHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "DoNotDisturbHandler", message);
}

void DoNotDisturbHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // text views
    m_doNotDisturbStateView = activity->findViewById("id:DoNotDisturbState");

    // initial view text
    activity->runOnUIThread([=]() {
        if (auto doNotDisturbStateView = m_doNotDisturbStateView.lock()) {
            doNotDisturbStateView->setText("");
        }
    });

    // doNotDisturbOn
    activity->registerObserver(Event::onDoNotDisturbOn, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onDoNotDisturbOn ");
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("DoNotDisturb On");
                console->printRuler();
            }
        });
        return doNotDisturbChanged(true);
    });

    // doNotDisturbOff
    activity->registerObserver(Event::onDoNotDisturbOff, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onDoNotDisturbOff");
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("DoNotDisturb Off");
                console->printRuler();
            }
        });
        return doNotDisturbChanged(false);
    });
}

}  // namespace alexa
}  // namespace sampleApp
