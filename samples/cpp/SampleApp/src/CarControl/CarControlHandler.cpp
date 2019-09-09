/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

CarControlHandler::CarControlHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler)
    : m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> CarControlHandler::getActivity() { 
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> CarControlHandler::getLoggerHandler() { 
    return m_loggerHandler;
}

void CarControlHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
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
    std::stringstream message;

    CarControlDataProvider::getBoolController(endpointId).setValue(true);

    message << "Entity   : Power Controller" << std::endl
            << "Endpoint : " << endpointId   << std::endl
            << "Action   : Turn on";

    showMessage(message.str());

    return true;
}

bool CarControlHandler::turnPowerControllerOff(const std::string& endpointId) {
    std::stringstream message;

    CarControlDataProvider::getBoolController(endpointId).setValue(false);

    message << "Entity   : Power Controller" << std::endl
            << "Endpoint : " << endpointId   << std::endl
            << "Action   : Turn off";

    showMessage(message.str());

    return true;
}

bool CarControlHandler::isPowerControllerOn(const std::string& endpointId, bool& isOn) {
    std::stringstream message;

    isOn = CarControlDataProvider::getBoolController(endpointId).getValue();
    
    message << "Entity   : Power Controller" << std::endl
            << "Endpoint : " << endpointId   << std::endl
            << "Action   : Query State"      << std::endl
            << "State    : " << (isOn ? "on" : "off");

    showMessage(message.str());

    return true;
}

// ToggleController
bool CarControlHandler::turnToggleControllerOn(const std::string& endpointId, const std::string& controllerId) {
    std::stringstream message;

    CarControlDataProvider::getBoolController(endpointId, controllerId).setValue(true);

    message << "Entity     : Toggle Controller" << std::endl
            << "Endpoint   : " << endpointId   << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Turn on";

    showMessage(message.str());

    return true;
}

bool CarControlHandler::turnToggleControllerOff(const std::string& endpointId, const std::string& controllerId) {
    std::stringstream message;

    CarControlDataProvider::getBoolController(endpointId, controllerId).setValue(false);

    message << "Entity     : Toggle Controller" << std::endl
            << "Endpoint   : " << endpointId   << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Turn off";

    showMessage(message.str());

    return true;
}

bool CarControlHandler::isToggleControllerOn(const std::string& endpointId, const std::string& controllerId, bool& isOn) {
    std::stringstream message;

    isOn = CarControlDataProvider::getBoolController(endpointId, controllerId).getValue();

    message << "Entity     : Toggle Controller" << std::endl
            << "Endpoint   : " << endpointId    << std::endl
            << "Controller : " << controllerId  << std::endl
            << "Action     : Query State"       << std::endl
            << "State      : " << (isOn ? "on" : "off");

    showMessage(message.str());

    return true;
}

// RangeController
bool CarControlHandler::setRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double value) {
    std::stringstream message;

    CarControlDataProvider::getRangeController(endpointId, controllerId).setValue(value);

    message << "Entity     : Range Controller" << std::endl
            << "Endpoint   : " << endpointId   << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Set Value"        << std::endl
            << "Value      : " << value;

    showMessage(message.str());

    return true;
}

bool CarControlHandler::adjustRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double delta) {
    std::stringstream message;

    auto value = CarControlDataProvider::getRangeController(endpointId, controllerId).getValue();
    CarControlDataProvider::getRangeController(endpointId, controllerId).adjustValue(delta);
    auto valueNew = CarControlDataProvider::getRangeController(endpointId, controllerId).getValue();

    message << "Entity     : Range Controller" << std::endl
            << "Endpoint   : " << endpointId   << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Adjust Value"     << std::endl
            << "Value      : " << value        << std::endl
            << "Delta      : " << delta        << std::endl
            << "New Value  : " << valueNew;


    showMessage(message.str());

    return true;
}

bool CarControlHandler::getRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double& value) {
    std::stringstream message;

    value = CarControlDataProvider::getRangeController(endpointId, controllerId).getValue();

    message << "Entity     : Range Controller" << std::endl
            << "Endpoint   : " << endpointId   << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Get Value"        << std::endl
            << "Value      : " << value;

    showMessage(message.str());

    return true;
}

// ModeController
bool CarControlHandler::setModeControllerValue(const std::string& endpointId, const std::string& controllerId, const std::string& value) {
    std::stringstream message;

    CarControlDataProvider::getModeController(endpointId, controllerId).setMode(value);

    message << "Entity     : Mode Controller"  << std::endl
            << "Endpoint   : " << endpointId   << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Set Mode"         << std::endl
            << "Value      : " << value;

    showMessage(message.str());

    return true;
}

bool CarControlHandler::adjustModeControllerValue(const std::string& endpointId, const std::string& controllerId, int delta) {
    std::stringstream message;

    auto value = CarControlDataProvider::getModeController(endpointId, controllerId).getMode();
    CarControlDataProvider::getModeController(endpointId, controllerId).adjustMode(delta);
    auto valueNew = CarControlDataProvider::getModeController(endpointId, controllerId).getMode();

    message << "Entity     : Mode Controller"  << std::endl
            << "Endpoint   : " << endpointId   << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Adjust Mode"      << std::endl
            << "Value      : " << value        << std::endl
            << "Delta      : " << delta        << std::endl
            << "New Value  : " << valueNew;

    showMessage(message.str());

    return true;
}

bool CarControlHandler::getModeControllerValue(const std::string& endpointId, const std::string& controllerId, std::string& value) {
    std::stringstream message;

    value = CarControlDataProvider::getModeController(endpointId, controllerId).getMode();

    message << "Entity     : Mode Controller"  << std::endl
            << "Endpoint   : " << endpointId   << std::endl
            << "Controller : " << controllerId << std::endl
            << "Action     : Get Mode"         << std::endl
            << "Value      : " << value;

    showMessage(message.str());
    return true;
}

bool CarControlHandler::checkConfiguration(const std::vector<json>& jsons, CarControlHandler::ConfigType type) {
    // Look for car control config
    for(auto const& j: jsons) {
        try {
            switch (type) {
                case ConfigType::LVC:
                    if (j.find("aace.localVoiceControl") != j.end() && 
                        j.find("aace.localSkillService") != j.end() ) {
                        return true;
                    }
                    break;
                case ConfigType::CAR:
                    if (j.find("aace.carControl") != j.end()) {
                        return true;
                    }
                    break;
            }
        } catch (json::exception &e) {
        }
    }

    return false;
}

} // namespace carControl
} // namespace sampleApp
