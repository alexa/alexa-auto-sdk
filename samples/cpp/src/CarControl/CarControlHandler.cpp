/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace carControl {

using namespace aace::carControl::assets;
using namespace aace::carControl::config;
using namespace aace::carControl;
using namespace aasb::message::carControl::carControl;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CarControlHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

static const std::string MODE_CONTROLLER("ModeController");
static const std::string POWER_CONTROLLER("PowerController");
static const std::string TOGGLE_CONTROLLER("ToggleController");
static const std::string RANGE_CONTROLLER("RangeController");

static const std::string AASB_TOPIC_CAR_CONTROL("CarControl");
static const std::string AASB_ACTION_SET_CONTROLLER_VALUE("SetControllerValue");
static const std::string AASB_ACTION_ADJUST_CONTROLLER_VALUE("AdjustControllerValue");

// aliases
using MessageBroker = aace::core::MessageBroker;

CarControlHandler::CarControlHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
    subscribeToAASBMessages();
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

void CarControlHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to SetControllerValue Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSetControllerValueMessage(message); },
        AASB_TOPIC_CAR_CONTROL,
        AASB_ACTION_SET_CONTROLLER_VALUE);

    log(logger::LoggerHandler::Level::INFO, "Subscribe to AdjustControllerValue Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAdjustControllerValueMessage(message); },
        AASB_TOPIC_CAR_CONTROL,
        AASB_ACTION_ADJUST_CONTROLLER_VALUE);
}

void CarControlHandler::handleSetControllerValueMessage(const std::string& message) {
    json msgJson = json::parse(message);
    log(logger::LoggerHandler::Level::INFO, "Received SetControllerValueMessage");

    std::string capabilityType = msgJson["payload"]["capabilityType"];
    if (capabilityType.compare("POWER") == 0) {
        SetPowerControllerValueMessage msg = json::parse(message);
        setPowerControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.turnOn);
    } else if (capabilityType.compare("TOGGLE") == 0) {
        SetToggleControllerValueMessage msg = json::parse(message);
        setToggleControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.turnOn);
    } else if (capabilityType.compare("RANGE") == 0) {
        SetRangeControllerValueMessage msg = json::parse(message);
        setRangeControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.value);
    } else if (capabilityType.compare("MODE") == 0) {
        SetModeControllerValueMessage msg = json::parse(message);
        setModeControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.value);
    } else {
        log(logger::LoggerHandler::Level::ERROR, "Unsupported controller type in message");
    }
}

void CarControlHandler::handleAdjustControllerValueMessage(const std::string& message) {
    json msgJson = json::parse(message);
    log(logger::LoggerHandler::Level::INFO, "Received AdjustControllerValueMessage");

    std::string capabilityType = msgJson["payload"]["capabilityType"];
    if (capabilityType.compare("RANGE") == 0) {
        AdjustRangeControllerValueMessage msg = json::parse(message);
        adjustRangeControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.delta);

    } else if (capabilityType.compare("MODE") == 0) {
        AdjustModeControllerValueMessage msg = json::parse(message);
        adjustModeControllerValue(msg.header.id, msg.payload.endpointId, msg.payload.instanceId, msg.payload.delta);
    } else {
        log(logger::LoggerHandler::Level::ERROR, "Unsupported controller type in message");
    }
}

void CarControlHandler::setPowerControllerValue(
    const std::string& messageId,
    const std::string& endpointId,
    bool value) {
    // Turn on/off the endpoint identified by "endpointId"
    CarControlDataProvider::getBoolController(endpointId).setValue(value);
    const std::string& name = value ? "TurnOn" : "TurnOff";

    std::stringstream console;
    console << "Endpoint        : " << endpointId << std::endl
            << "Capability      : " << POWER_CONTROLLER << std::endl
            << "Action          : " << name << std::endl
            << "AASB Message Id : " << messageId;
    showMessage(console.str());

    // Publish the reply message indicating the endpoint was successfully turned on/off.
    SetControllerValueMessageReply msg;
    bool success = true;
    msg.header.messageDescription.replyToId = messageId;
    msg.payload.success = success;

    m_messageBroker->publish(msg.toString());
}

void CarControlHandler::setToggleControllerValue(
    const std::string& messageId,
    const std::string& endpointId,
    const std::string& instanceId,
    bool value) {
    // For the endpoint identified by "endpointId", turn on/off
    // the property instance identified by "instanceId".
    CarControlDataProvider::getBoolController(endpointId, instanceId).setValue(value);
    const std::string& name = value ? "TurnOn" : "TurnOff";

    std::stringstream console;
    console << "Endpoint        : " << endpointId << std::endl
            << "Capability      : " << TOGGLE_CONTROLLER << std::endl
            << "Instance        : " << instanceId << std::endl
            << "Action          : " << name << std::endl
            << "AASB Message Id : " << messageId;
    showMessage(console.str());

    // Publish the reply message indicating the property instance for the endpoint was successfully turned on/off.
    SetControllerValueMessageReply msg;
    bool success = true;
    msg.header.messageDescription.replyToId = messageId;
    msg.payload.success = success;

    m_messageBroker->publish(msg.toString());
}

void CarControlHandler::setRangeControllerValue(
    const std::string& messageId,
    const std::string& endpointId,
    const std::string& instanceId,
    double value) {
    // For the endpoint identified by "endpointId", set the property instance
    // identified by "instanceId" to the specified "value".
    CarControlDataProvider::getRangeController(endpointId, instanceId).setValue(value);

    std::stringstream console;
    console << "Endpoint        : " << endpointId << std::endl
            << "Capability      : " << RANGE_CONTROLLER << std::endl
            << "Instance        : " << instanceId << std::endl
            << "Action          : Set Value" << std::endl
            << "Value           : " << value << std::endl
            << "AASB Message Id : " << messageId;
    showMessage(console.str());

    // Publish the reply message indicating the range value was successfully set
    // for the property instance for the endpoint.
    SetControllerValueMessageReply msg;
    bool success = true;
    msg.header.messageDescription.replyToId = messageId;
    msg.payload.success = success;

    m_messageBroker->publish(msg.toString());
}

void CarControlHandler::adjustRangeControllerValue(
    const std::string& messageId,
    const std::string& endpointId,
    const std::string& instanceId,
    double delta) {
    // For the endpoint identified by "endpointId", adjust the property instance
    // identified by "instanceId" by the specified "delta" value.
    auto value = CarControlDataProvider::getRangeController(endpointId, instanceId).getValue();
    CarControlDataProvider::getRangeController(endpointId, instanceId).adjustValue(delta);
    auto valueNew = CarControlDataProvider::getRangeController(endpointId, instanceId).getValue();

    std::stringstream console;
    console << "Endpoint        : " << endpointId << std::endl
            << "Capability      : " << RANGE_CONTROLLER << std::endl
            << "Instance        : " << instanceId << std::endl
            << "Action          : Adjust Value" << std::endl
            << "Current Value   : " << value << std::endl
            << "Delta           : " << delta << std::endl
            << "New Value       : " << valueNew << std::endl
            << "AASB Message Id : " << messageId;
    showMessage(console.str());

    // Publish the reply message indicating the range setting was successfully adjusted
    // for the property instance for the endpoint.
    AdjustControllerValueMessageReply msg;
    bool success = true;
    msg.header.messageDescription.replyToId = messageId;
    msg.payload.success = success;

    m_messageBroker->publish(msg.toString());
}

void CarControlHandler::setModeControllerValue(
    const std::string& messageId,
    const std::string& endpointId,
    const std::string& instanceId,
    const std::string& value) {
    // For the endpoint identified by "endpointId", set the property instance
    // identified by "instanceId" to the specified "value".
    CarControlDataProvider::getModeController(endpointId, instanceId).setMode(value);

    std::stringstream console;
    console << "Endpoint        : " << endpointId << std::endl
            << "Capability      : " << MODE_CONTROLLER << std::endl
            << "Instance        : " << instanceId << std::endl
            << "Action          : Set Mode" << std::endl
            << "Value           : " << value << std::endl
            << "AASB Message Id : " << messageId;
    showMessage(console.str());

    // Publish the reply message indicating the mode value was successfully set
    // for the property instance for the endpoint.
    SetControllerValueMessageReply msg;
    bool success = true;
    msg.header.messageDescription.replyToId = messageId;
    msg.payload.success = success;

    m_messageBroker->publish(msg.toString());
}
void CarControlHandler::adjustModeControllerValue(
    const std::string& messageId,
    const std::string& endpointId,
    const std::string& instanceId,
    int delta) {
    // For the endpoint identified by "endpointId", adjust the property instance
    // identified by "instanceId" by the specified "delta" value.
    auto value = CarControlDataProvider::getModeController(endpointId, instanceId).getMode();
    CarControlDataProvider::getModeController(endpointId, instanceId).adjustMode(delta);
    auto valueNew = CarControlDataProvider::getModeController(endpointId, instanceId).getMode();

    std::stringstream console;
    console << "Endpoint        : " << endpointId << std::endl
            << "Capability      : " << MODE_CONTROLLER << std::endl
            << "Instance        : " << instanceId << std::endl
            << "Action          : Adjust Mode" << std::endl
            << "Current Value   : " << value << std::endl
            << "Delta           : " << delta << std::endl
            << "New Value       : " << valueNew << std::endl
            << "AASB Message Id : " << messageId;
    showMessage(console.str());

    // Publish the reply message indicating the mode value was successfully adjusted
    // for the property instance for the endpoint.
    AdjustControllerValueMessageReply msg;
    bool success = true;
    msg.header.messageDescription.replyToId = messageId;
    msg.payload.success = success;

    m_messageBroker->publish(msg.toString());
}

bool CarControlHandler::checkConfiguration(const std::vector<json>& jsons) {
    // Look for car control config
    for (auto const& j : jsons) {
        try {
            if (j.find("aace.carControl") != j.end()) {
                return true;
            }
        } catch (json::exception& e) {
        }
    }

    return false;
}

}  // namespace carControl
}  // namespace sampleApp
