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

#include "SampleApp/Location/LocationProviderHandler.h"
#include "SampleApp/ApplicationContext.h"

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace location {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocationProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LocationProviderHandler::LocationProviderHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> LocationProviderHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> LocationProviderHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::location::LocationProvider interface

aace::location::Location LocationProviderHandler::getLocation() {
    double latitude = 0.0, longitude = 0.0;
    if (auto activity = m_activity.lock()) {
        auto applicationContext = activity->getApplicationContext();
        auto value = applicationContext->getMenuItemValue("LOCATION", nullptr);
        if (value.is_object() && value.count("latitude") && value.count("longitude")) {
            latitude = value.at("latitude").get<double>();
            longitude = value.at("longitude").get<double>();
        }
    }

    if (latitude == 0.0 && longitude == 0.0) {
        latitude = aace::location::Location::UNDEFINED;
        longitude = aace::location::Location::UNDEFINED;
    }

    if (auto console = m_console.lock()) {
        console->printLine("Location", latitude, longitude);
    }
    return aace::location::Location(latitude, longitude);
}

// private

void LocationProviderHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "LocationProviderHandler", message);
}

void LocationProviderHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

}  // namespace location
}  // namespace sampleApp
