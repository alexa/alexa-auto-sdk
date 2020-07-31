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

#include <sstream>
#include <fstream>

#include <AACE/CarControl/CarControlAssets.h>

#include <SampleApp/ApplicationContext.h>
#include <SampleApp/CarControl/CarControlHandler.h>
#include <SampleApp/CarControl/CarControlDataProvider.h>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace carControl {

using namespace aace::carControl::assets;
using namespace aace::carControl::config;
using namespace aace::carControl;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CarControlHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

static const std::string MODE_CONTROLLER("ModeController");
static const std::string POWER_CONTROLLER("PowerController");
static const std::string TOGGLE_CONTROLLER("ToggleController");
static const std::string RANGE_CONTROLLER("RangeController");

CarControlHandler::CarControlHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> CarControlHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> CarControlHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void CarControlHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "CarControlHandler", message);
}

void CarControlHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

void CarControlHandler::showMessage(std::string message) {
    log(logger::LoggerHandler::Level::INFO, message);
    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine(message);
        console->printRuler();
    }
}

// PowerController
bool CarControlHandler::turnPowerControllerOn(const std::string& endpointId) {
    CarControlDataProvider::getBoolController(endpointId).setValue(true);

    std::stringstream logMessage;
    logMessage << POWER_CONTROLLER << ",endpoint=" << endpointId << ",name=TurnOn";
    log(logger::LoggerHandler::Level::INFO, logMessage.str());

    std::stringstream console;
    console << "Entity   : Power Controller" << std::endl
            << "Endpoint : " << endpointId << std::endl
            << "Action   : Turn on";
    showMessage(console.str());

    return true;
}

bool CarControlHandler::turnPowerControllerOff(const std::string& endpointId) {
    CarControlDataProvider::getBoolController(endpointId).setValue(false);

    std::stringstream logMessage;
    logMessage << POWER_CONTROLLER << ",endpoint=" << endpointId << ",name=TurnOff";
    log(logger::LoggerHandler::Level::INFO, logMessage.str());

    std::stringstream console;
    console << "Entity   : Power Controller" << std::endl
            << "Endpoint : " << endpointId << std::endl
            << "Action   : Turn off";

    showMessage(console.str());

    return true;
}

bool CarControlHandler::isPowerControllerOn(const std::string& endpointId, bool& isOn) {
    isOn = CarControlDataProvider::getBoolController(endpointId).getValue();

    std::stringstream console;
    console << "Entity   : Power Controller" << std::endl
            << "Endpoint : " << endpointId << std::endl
            << "Action   : Query State" << std::endl
            << "State    : " << (isOn ? "on" : "off");
    showMessage(console.str());

    return true;
}

// ToggleController
bool CarControlHandler::turnToggleControllerOn(const std::string& endpointId, const std::string& controllerId) {
    CarControlDataProvider::getBoolController(endpointId, controllerId).setValue(true);

    std::stringstream logMessage;
    logMessage << TOGGLE_CONTROLLER << ",endpoint=" << endpointId << ",name=TurnOn,instance=" << controllerId;
    log(logger::LoggerHandler::Level::INFO, logMessage.str());

    std::stringstream console;
    console << "Entity     : Toggle Controller" << std::endl
            << "Endpoint   : " << endpointId << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Turn on";
    showMessage(console.str());

    return true;
}

bool CarControlHandler::turnToggleControllerOff(const std::string& endpointId, const std::string& controllerId) {
    CarControlDataProvider::getBoolController(endpointId, controllerId).setValue(false);

    std::stringstream logMessage;
    logMessage << TOGGLE_CONTROLLER << ",endpoint=" << endpointId << ",name=TurnOff,instance=" << controllerId;
    log(logger::LoggerHandler::Level::INFO, logMessage.str());

    std::stringstream console;
    console << "Entity     : Toggle Controller" << std::endl
            << "Endpoint   : " << endpointId << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Turn off";
    showMessage(console.str());

    return true;
}

bool CarControlHandler::isToggleControllerOn(
    const std::string& endpointId,
    const std::string& controllerId,
    bool& isOn) {
    isOn = CarControlDataProvider::getBoolController(endpointId, controllerId).getValue();

    std::stringstream console;
    console << "Entity     : Toggle Controller" << std::endl
            << "Endpoint   : " << endpointId << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Query State" << std::endl
            << "State      : " << (isOn ? "on" : "off");
    showMessage(console.str());

    return true;
}

// RangeController
bool CarControlHandler::setRangeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    double value) {
    CarControlDataProvider::getRangeController(endpointId, controllerId).setValue(value);

    std::stringstream logMessage;
    logMessage << RANGE_CONTROLLER << ",endpoint=" << endpointId << ",name=SetRangeValue"
               << ",instance" << controllerId << ",rangeValue" << value;
    log(logger::LoggerHandler::Level::INFO, logMessage.str());

    std::stringstream console;
    console << "Entity     : Range Controller" << std::endl
            << "Endpoint   : " << endpointId << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Set Value" << std::endl
            << "Value      : " << value;
    showMessage(console.str());

    return true;
}

bool CarControlHandler::adjustRangeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    double delta) {
    auto value = CarControlDataProvider::getRangeController(endpointId, controllerId).getValue();
    CarControlDataProvider::getRangeController(endpointId, controllerId).adjustValue(delta);
    auto valueNew = CarControlDataProvider::getRangeController(endpointId, controllerId).getValue();

    std::stringstream logMessage;
    logMessage << RANGE_CONTROLLER << ",endpoint=" << endpointId << ",name=AdjustRangeValue"
               << ",instance" << controllerId << ",rangeValueDelta" << delta;
    log(logger::LoggerHandler::Level::INFO, logMessage.str());

    std::stringstream console;
    console << "Entity     : Range Controller" << std::endl
            << "Endpoint   : " << endpointId << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Adjust Value" << std::endl
            << "Value      : " << value << std::endl
            << "Delta      : " << delta << std::endl
            << "New Value  : " << valueNew;
    showMessage(console.str());

    return true;
}

bool CarControlHandler::getRangeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    double& value) {
    value = CarControlDataProvider::getRangeController(endpointId, controllerId).getValue();

    std::stringstream console;
    console << "Entity     : Range Controller" << std::endl
            << "Endpoint   : " << endpointId << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Get Value" << std::endl
            << "Value      : " << value;
    showMessage(console.str());

    return true;
}

// ModeController
bool CarControlHandler::setModeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    const std::string& value) {
    CarControlDataProvider::getModeController(endpointId, controllerId).setMode(value);

    std::stringstream logMessage;
    logMessage << MODE_CONTROLLER << ",endpoint=" << endpointId << ",name=SetMode"
               << ",instance" << controllerId << ",mode" << value;
    log(logger::LoggerHandler::Level::INFO, logMessage.str());

    std::stringstream console;
    console << "Entity     : Mode Controller" << std::endl
            << "Endpoint   : " << endpointId << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Set Mode" << std::endl
            << "Value      : " << value;
    showMessage(console.str());

    return true;
}

bool CarControlHandler::adjustModeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    int delta) {
    auto value = CarControlDataProvider::getModeController(endpointId, controllerId).getMode();
    CarControlDataProvider::getModeController(endpointId, controllerId).adjustMode(delta);
    auto valueNew = CarControlDataProvider::getModeController(endpointId, controllerId).getMode();

    std::stringstream logMessage;
    logMessage << MODE_CONTROLLER << ",endpoint=" << endpointId << ",name=AdjustMode"
               << ",instance" << controllerId << ",modeDelta" << delta;
    log(logger::LoggerHandler::Level::INFO, logMessage.str());

    std::stringstream console;
    console << "Entity     : Mode Controller" << std::endl
            << "Endpoint   : " << endpointId << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Adjust Mode" << std::endl
            << "Value      : " << value << std::endl
            << "Delta      : " << delta << std::endl
            << "New Value  : " << valueNew;
    showMessage(console.str());

    return true;
}

bool CarControlHandler::getModeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    std::string& value) {
    value = CarControlDataProvider::getModeController(endpointId, controllerId).getMode();

    std::stringstream console;
    console << "Entity     : Mode Controller" << std::endl
            << "Endpoint   : " << endpointId << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Get Mode" << std::endl
            << "Value      : " << value;
    showMessage(console.str());

    return true;
}

bool CarControlHandler::checkConfiguration(const std::vector<json>& jsons, CarControlHandler::ConfigType type) {
    // Look for car control config
    for (auto const& j : jsons) {
        try {
            switch (type) {
                case ConfigType::LVC:
                    if (j.find("aace.localVoiceControl") != j.end() && j.find("aace.localSkillService") != j.end()) {
                        return true;
                    }
                    break;
                case ConfigType::CAR:
                    if (j.find("aace.carControl") != j.end()) {
                        return true;
                    }
                    break;
            }
        } catch (json::exception& e) {
        }
    }

    return false;
}

}  // namespace carControl
}  // namespace sampleApp
