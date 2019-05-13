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

#include "SampleApp/CarControl/ClimateControlHandler.h"
#include "SampleApp/ApplicationContext.h"

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace carControl {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ClimateControlHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ClimateControlHandler::ClimateControlHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler)
    : m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> ClimateControlHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> ClimateControlHandler::getLoggerHandler() { return m_loggerHandler; }

// aace::location::LocationProvider interface

// private

void ClimateControlHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "ClimateControlHandler", message);
}

void ClimateControlHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

bool ClimateControlHandler::turnClimateControlOn() {
    m_cc = true;
    log(logger::LoggerHandler::Level::INFO, "Power On");
    return true;
}

bool ClimateControlHandler::turnClimateControlOff() {
    m_cc = false;
    log(logger::LoggerHandler::Level::INFO, "Power Off");
    return true;
}

bool ClimateControlHandler::isClimateControlOn() { return m_cc; }

// Air Conditioning

bool ClimateControlHandler::turnAirConditioningOn() {
    m_ac = true;
    log(logger::LoggerHandler::Level::INFO, "Air Conditioning On");
    return true;
}
bool ClimateControlHandler::turnAirConditioningOff() {
    m_ac = false;
    log(logger::LoggerHandler::Level::INFO, "Air Conditioning Off");
    return true;
}
bool ClimateControlHandler::isAirConditioningOn() { return m_ac; }

bool ClimateControlHandler::setAirConditioningMode(aace::carControl::ClimateControlInterface::AirConditioningMode mode) {
    m_ac_mode = mode;
    if (mode == aace::carControl::ClimateControlInterface::AirConditioningMode::AUTO) {
        log(logger::LoggerHandler::Level::INFO, "Auto On");
    } else {
        log(logger::LoggerHandler::Level::INFO, "Auto Off");
    }
    return true;
}

aace::carControl::ClimateControlInterface::AirConditioningMode ClimateControlHandler::getAirConditioningMode() { return m_ac_mode; }

// Fan

bool ClimateControlHandler::turnFanOn(aace::carControl::ClimateControlInterface::FanZone zone) {
    m_fan = true;
    log(logger::LoggerHandler::Level::INFO, "Fan On");
    return true;
}

bool ClimateControlHandler::turnFanOff(aace::carControl::ClimateControlInterface::FanZone zone) {
    m_fan = false;
    log(logger::LoggerHandler::Level::INFO, "Fan Off");
    return true;
}

bool ClimateControlHandler::isFanOn(aace::carControl::ClimateControlInterface::FanZone zone) { return m_fan; }

bool ClimateControlHandler::setFanSpeed(aace::carControl::ClimateControlInterface::FanZone zone, double value) {
    m_fanSpeed = value;
    log(logger::LoggerHandler::Level::INFO, "Fan Speed = " + std::to_string(m_fanSpeed));
    return true;
}

bool ClimateControlHandler::adjustFanSpeed(aace::carControl::ClimateControlInterface::FanZone zone, double delta) {
    m_fanSpeed += delta;
    log(logger::LoggerHandler::Level::INFO, "Fan Speed = " + std::to_string(m_fanSpeed));
    return true;
}

double ClimateControlHandler::getFanSpeed(aace::carControl::ClimateControlInterface::FanZone zone) { return m_fanSpeed; }

// Temperature

bool ClimateControlHandler::setTemperature(aace::carControl::ClimateControlInterface::TemperatureZone zone, double value) {
    m_temperature = value;
    log(logger::LoggerHandler::Level::INFO, "Temperature = " + std::to_string(m_temperature));
    return true;
}

bool ClimateControlHandler::adjustTemperature(aace::carControl::ClimateControlInterface::TemperatureZone zone, double delta) {
    m_temperature += delta;
    log(logger::LoggerHandler::Level::INFO, "Temperature = " + std::to_string(m_temperature));
    return true;
}

double ClimateControlHandler::getTemperature(aace::carControl::ClimateControlInterface::TemperatureZone zone) { return m_temperature; }

} // namespace carControl
} // namespace sampleApp
