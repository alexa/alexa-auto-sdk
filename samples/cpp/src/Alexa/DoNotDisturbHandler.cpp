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

#include "SampleApp/Alexa/DoNotDisturbHandler.h"

#include <AASB/Message/Alexa/DoNotDisturb/SetDoNotDisturbMessage.h>
#include <AASB/Message/Alexa/DoNotDisturb/DoNotDisturbChangedMessage.h>

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::doNotDisturb;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DoNotDisturbHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// aliases
using MessageBroker = aace::core::MessageBroker;

DoNotDisturbHandler::DoNotDisturbHandler(
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

std::weak_ptr<Activity> DoNotDisturbHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> DoNotDisturbHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void DoNotDisturbHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to SetDoNotDisturb Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSetDoNotDisturbMessage(message); },
        SetDoNotDisturbMessage::topic(),
        SetDoNotDisturbMessage::action());
}

void DoNotDisturbHandler::handleSetDoNotDisturbMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received SetDoNotDisturbMessage");
    SetDoNotDisturbMessage msg = json::parse(message);
    setDoNotDisturb(msg.payload.doNotDisturb);
}

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

void DoNotDisturbHandler::doNotDisturbChanged(bool doNotDisturb) {
    // Publish the "DoNotDisturbChanged" message
    DoNotDisturbChangedMessage msg;
    msg.payload.doNotDisturb = doNotDisturb;
    m_messageBroker->publish(msg.toString());
}

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
        doNotDisturbChanged(true);
        return true;
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
        doNotDisturbChanged(false);
        return true;
    });
}

}  // namespace alexa
}  // namespace sampleApp
