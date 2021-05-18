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

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DeviceSetupHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

DeviceSetupHandler::DeviceSetupHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    setupUI();
}

std::weak_ptr<Activity> DeviceSetupHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> DeviceSetupHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::DeviceSetup interface

void DeviceSetupHandler::setupCompletedResponse(StatusCode statusCode) {
    std::stringstream ss;
    ss << "setupCompletedResponse " << statusCode;
    log(logger::LoggerHandler::Level::INFO, ss.str());
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
