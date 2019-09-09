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
#include "SampleApp/ApplicationContext.h"

#include <fstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

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
    m_dummyNavigationState = "";
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
    return clearNavigationState();
}

// private

std::string NavigationHandler::getNavigationState() {
    log(logger::LoggerHandler::Level::INFO, "getNavigationState");
    return m_dummyNavigationState;
}

// Sample App Events

bool NavigationHandler::loadNavigationState(const std::string &filepath) {
    std::ifstream i(filepath);
    json j;
    i >> j;

    if (j.is_object()) {
        auto waypoints = j.find("waypoints");
        if(waypoints != j.end() && waypoints->is_array()) {
            m_dummyNavigationState = j.dump(); // json > std::string
            log(logger::LoggerHandler::Level::INFO, m_dummyNavigationState);
            return true;
        } else {
            log(logger::LoggerHandler::Level::ERROR, "Cannot find json array with key of 'waypoints' ");
            return false;
        }
    } else {
        log(logger::LoggerHandler::Level::ERROR, "Navigation state data invalid");
        return false;
    }
}

bool NavigationHandler::clearNavigationState() {
    m_dummyNavigationState = "";
    log(logger::LoggerHandler::Level::INFO, "Navigation state data cleared");
    return true;
}

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

    

    activity->registerObserver(Event::onLoadNavigationState, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onLoadNavigationState");
        if (auto console = m_console.lock()) {
            console->printLine("Loading from" + value);
        }
        return loadNavigationState(value);
    });

    activity->registerObserver(Event::onClearNavigationState, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onClearNavigationState");
        if (auto console = m_console.lock()) {
            console->printLine("Clearing nav state");
        }
        return clearNavigationState();
    });
}

} // namespace navigation
} // namespace sampleApp
