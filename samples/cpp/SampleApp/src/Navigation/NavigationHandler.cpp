/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Navigation/NavigationHandler.h"

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace navigation {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NavigationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

NavigationHandler::NavigationHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler)
    : m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> NavigationHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> NavigationHandler::getLoggerHandler() { return m_loggerHandler; }

// aace::navigation::Navigation interface

bool NavigationHandler::setDestination(const std::string &payload) {
    log(logger::LoggerHandler::Level::INFO, "setDestination:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            card->set(payload, View::Type::Navigation);
        }
    });
    return true;
}

bool NavigationHandler::cancelNavigation() {
    log(logger::LoggerHandler::Level::INFO, "cancelNavigation");
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            card->clear(View::Type::Navigation);
        }
    });
    return true;
}

// private

void NavigationHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "NavigationHandler", message);
}

void NavigationHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

} // namespace navigation
} // namespace sampleApp
