/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Alexa/DeviceSetupHandler.h"

#include <AASB/Message/Alexa/DeviceSetup/SetupCompletedResponseMessage.h>
#include <AASB/Message/Alexa/DeviceSetup/SetupCompletedMessage.h>

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::deviceSetup;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DeviceSetupHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// aliases
using MessageBroker = aace::core::MessageBroker;

DeviceSetupHandler::DeviceSetupHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr) && (m_messageBroker != nullptr))
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> DeviceSetupHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> DeviceSetupHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void DeviceSetupHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to SetupCompletedResponse Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSetupCompletedResponseMessage(message); },
        SetupCompletedResponseMessage::topic(),
        SetupCompletedResponseMessage::action());
}

void DeviceSetupHandler::handleSetupCompletedResponseMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received SetupCompletedResponseMessage");
    SetupCompletedResponseMessage msg = json::parse(message);
    setupCompletedResponse(msg.payload.statusCode);
}

void DeviceSetupHandler::setupCompletedResponse(StatusCode statusCode) {
    std::stringstream ss;
    switch (statusCode) {
        case StatusCode::SUCCESS:
            ss << "setupCompletedResponse SUCCESS";
            break;
        case StatusCode::FAIL:
            ss << "setupCompletedResponse FAIL";
            break;
    }
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

void DeviceSetupHandler::setupCompleted() {
    // Publish the "SetupCompleted" message
    SetupCompletedMessage msg;
    m_messageBroker->publish(msg.toString());
}

// private

void DeviceSetupHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "DeviceSetupHandler", message);
}

void DeviceSetupHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // device setup completed
    activity->registerObserver(Event::onDeviceSetupCompleted, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onDeviceSetupCompleted");
        setupCompleted();
        return true;
    });
}

}  // namespace alexa
}  // namespace sampleApp
