/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Alexa/GlobalPresetHandler.h"

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  GlobalPresetHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

GlobalPresetHandler::GlobalPresetHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    setupUI();
}

std::weak_ptr<Activity> GlobalPresetHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> GlobalPresetHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::GlobalPreset interface

void GlobalPresetHandler::setGlobalPreset(int preset) {
    log(logger::LoggerHandler::Level::INFO, std::to_string(preset));
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Set global preset: ", std::to_string(preset));
            console->printRuler();
        }
    });
}

// private

void GlobalPresetHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "GlobalPresetHandler", message);
}

void GlobalPresetHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

}  // namespace alexa
}  // namespace sampleApp
