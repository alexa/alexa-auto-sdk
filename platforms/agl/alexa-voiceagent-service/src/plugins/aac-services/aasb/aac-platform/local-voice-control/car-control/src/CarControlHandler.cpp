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
#include "CarControlHandler.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <aasb/Consts.h>

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;
using namespace aasb::bridge;

namespace aasb {
namespace carControl {

const std::string TAG = "aasb::carControl::CarControlHandler";
const std::chrono::milliseconds CALL_WAIT_TIMEOUT(2000);

std::shared_ptr<CarControlHandler> CarControlHandler::create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    auto handler =
            std::shared_ptr<CarControlHandler>(
                    new CarControlHandler(logger, responseDispatcher));

    return handler;
}

CarControlHandler::CarControlHandler(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) :
        m_logger(logger),
        m_responseDispatcher(responseDispatcher),
        m_callIsClimateControlOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callIsClimateControlSyncOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callIsAirRecirculationOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callIsAirConditionerOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callGetAirConditionerMode(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callIsHeaterOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callGetHeaterTemperature(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callIsFanOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callGetFanSpeed(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callIsVentOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callGetVentPosition(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callIsWindowDefrosterOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callIsLightOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callGetLightColor(logger, responseDispatcher, CALL_WAIT_TIMEOUT) {
}

bool CarControlHandler::turnClimateControlOn(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_CLIMATE_TURN_ON, "");
}

bool CarControlHandler::turnClimateControlOff(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_CLIMATE_TURN_OFF, "");
}

bool CarControlHandler::isClimateControlOn(const std::string& zoneId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(zoneId, isOn, ACTION_CARCONTROL_CLIMATE_IS_ON, m_callIsClimateControlOn, "");
}

bool CarControlHandler::turnClimateSyncOn(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_CLIMATE_SYNC_TURN_ON, "");
}

bool CarControlHandler::turnClimateSyncOff(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_CLIMATE_SYNC_TURN_OFF, "");
}

bool CarControlHandler::isClimateSyncOn(const std::string& zoneId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(zoneId, isOn, ACTION_CARCONTROL_CLIMATE_SYNC_IS_ON, m_callIsClimateControlSyncOn, "");
}

bool CarControlHandler::turnAirRecirculationOn(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_AIR_RECIRCULATION_TURN_ON, "");
}

bool CarControlHandler::turnAirRecirculationOff(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_AIR_RECIRCULATION_TURN_OFF, "");
}

bool CarControlHandler::isAirRecirculationOn(const std::string& zoneId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(zoneId, isOn, ACTION_CARCONTROL_AIR_RECIRCULATION_IS_ON, m_callIsAirRecirculationOn, "");
}

// AirConditioner
bool CarControlHandler::turnAirConditionerOn(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_AC_TURN_ON, "");
}

bool CarControlHandler::turnAirConditionerOff(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_AC_TURN_OFF,"");
}

bool CarControlHandler::isAirConditionerOn(const std::string& zoneId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(zoneId, isOn, ACTION_CARCONTROL_AC_IS_ON, m_callIsAirConditionerOn, "");
}

bool CarControlHandler::setAirConditionerMode(const std::string& zoneId, AirConditionerMode mode) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());

    std::string airConditionerModeString = airConditionerModeToString(mode);
    document.AddMember(
        "mode",
        rapidjson::Value().SetString(airConditionerModeString.c_str(), airConditionerModeString.length()),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    responseDispatcher->sendDirective(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_AC_SET_MODE,
            buffer.GetString());
    return true;
}
    
bool CarControlHandler::getAirConditionerMode(const std::string& zoneId, AirConditionerMode& mode) {
    std::string responseStr;

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    bool callResult = m_callGetAirConditionerMode.makeCallAndWaitForResponse(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_AC_GET_MODE,
            buffer.GetString(), responseStr);
    if (callResult) {
        rapidjson::Document document;
        rapidjson::ParseResult parseResult = document.Parse(responseStr.c_str());
        if (!parseResult) {
            m_logger->log(Level::WARN, TAG, rapidjson::GetParseError_En(parseResult.Code()));
            return false;
        }

        auto root = document.GetObject();
        bool status = false;
        if (root.HasMember(JSON_ATTR_CARCONTROL_AC_MODE.c_str()) &&
                root[JSON_ATTR_CARCONTROL_AC_MODE.c_str()].IsString()) {
            auto modeString = root[JSON_ATTR_CARCONTROL_AC_MODE.c_str()].GetString();
            mode = airConditionerModeFromString(modeString);
        } else {
            m_logger->log(Level::WARN, TAG, "Missing attribute: " +
                    JSON_ATTR_CARCONTROL_AC_MODE);
            return false;
        }

        return true;
    } else {
        m_logger->log(Level::WARN, TAG, "Failed to obtain response for " + ACTION_CARCONTROL_AC_GET_MODE);
        return false;
    }
}

// Heater
bool CarControlHandler::turnHeaterOn(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_HEATER_TURN_ON, "");
}

bool CarControlHandler::turnHeaterOff(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_HEATER_TURN_OFF, "");
}

bool CarControlHandler::isHeaterOn(const std::string& zoneId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(zoneId, isOn, ACTION_CARCONTROL_HEATER_IS_ON, m_callIsHeaterOn, "");
}

bool CarControlHandler::setHeaterTemperature(const std::string& zoneId, double value) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return setValue(zoneId, value, ACTION_CARCONTROL_HEATER_SET_TEMPERATURE);
}

bool CarControlHandler::adjustHeaterTemperature(const std::string& zoneId, double delta) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return adjustValue(zoneId, delta, ACTION_CARCONTROL_HEATER_ADJUST_TEMPERATURE);
}

bool CarControlHandler::getHeaterTemperature(const std::string& zoneId, double& value) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return getValue(zoneId, value, ACTION_CARCONTROL_HEATER_GET_TEMPERATURE, m_callGetHeaterTemperature);
}

// Fan
bool CarControlHandler::turnFanOn(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_FAN_TURN_ON, "");
}

bool CarControlHandler::turnFanOff(const std::string& zoneId){
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_FAN_TURN_OFF, "");
}

bool CarControlHandler::isFanOn(const std::string& zoneId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(zoneId, isOn, ACTION_CARCONTROL_FAN_IS_ON, m_callIsFanOn, "");
}

bool CarControlHandler::setFanSpeed(const std::string& zoneId, double value) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return setValue(zoneId, value, ACTION_CARCONTROL_FAN_SET_SPEED);
}

bool CarControlHandler::adjustFanSpeed(const std::string& zoneId, double delta) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return adjustValue(zoneId, delta, ACTION_CARCONTROL_FAN_ADJUST_SPEED);
}

bool CarControlHandler::getFanSpeed(const std::string& zoneId, double& value) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return getValue(zoneId, value, ACTION_CARCONTROL_FAN_GET_SPEED, m_callGetFanSpeed);
}

// Vent
bool CarControlHandler::turnVentOn(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_VENT_TURN_ON, "");
}

bool CarControlHandler::turnVentOff(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_VENT_TURN_OFF, "");
}

bool CarControlHandler::isVentOn(const std::string& zoneId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(zoneId, isOn, ACTION_CARCONTROL_VENT_IS_ON, m_callIsVentOn, "");
}

bool CarControlHandler::setVentPosition(const std::string& zoneId, CarControl::VentPosition value) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());

    auto ventPositionString = ventPositionToString(value);
    document.AddMember(
        "ventPosition",
        rapidjson::Value().SetString(ventPositionString.c_str(), ventPositionString.length()),
        document.GetAllocator());    
    
    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    responseDispatcher->sendDirective(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_VENT_SET_POSITION,
            buffer.GetString());
    return true;
}

bool CarControlHandler::getVentPosition(const std::string& zoneId, CarControl::VentPosition& vent) {
    std::string responseStr;

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());
    
    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    bool callResult = m_callGetVentPosition.makeCallAndWaitForResponse(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_VENT_GET_POSITION,
            buffer.GetString(), responseStr);
    if (callResult) {
        rapidjson::Document document;
        rapidjson::ParseResult parseResult = document.Parse(responseStr.c_str());
        if (!parseResult) {
            m_logger->log(Level::WARN, TAG, rapidjson::GetParseError_En(parseResult.Code()));
            return false;
        }
        auto root = document.GetObject();

        std::string ventPositionString;

        if (root.HasMember(JSON_ATTR_CARCONTROL_VENT_POSITION.c_str()) &&
                root[JSON_ATTR_CARCONTROL_VENT_POSITION.c_str()].IsString()) {
            ventPositionString = root[JSON_ATTR_CARCONTROL_VENT_POSITION.c_str()].GetString();
        } else {
            m_logger->log(Level::WARN, TAG, "getVentPosition: missing attribute: " +
                    JSON_ATTR_CARCONTROL_VENT_POSITION);
            return false;
        }

        m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
        vent = ventPositionFromString(ventPositionString);

        return true;
    } else {
        m_logger->log(Level::WARN, TAG, "Failed to obtain response for vent position");
        return false;
    }
}

// Defroster
bool CarControlHandler::turnWindowDefrosterOn(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_WINDOW_DEFROSTER_TURN_ON, "");
}

bool CarControlHandler::turnWindowDefrosterOff(const std::string& zoneId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_WINDOW_DEFROSTER_TURN_OFF, "");
}

bool CarControlHandler::isWindowDefrosterOn(const std::string& zoneId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(zoneId, isOn, ACTION_CARCONTROL_WINDOW_DEFROSTER_IS_ON, m_callIsWindowDefrosterOn, "");
}

// Lights
bool CarControlHandler::turnLightOn(const std::string& zoneId, aace::carControl::CarControl::LightType type) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_LIGHT_TURN_ON, lightTypeToString(type));
}

bool CarControlHandler::turnLightOff(const std::string& zoneId, aace::carControl::CarControl::LightType type) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(zoneId, ACTION_CARCONTROL_LIGHT_TURN_OFF, lightTypeToString(type));
}

bool CarControlHandler::isLightOn(const std::string& zoneId, aace::carControl::CarControl::LightType type, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(zoneId, isOn, ACTION_CARCONTROL_LIGHT_IS_ON, m_callIsWindowDefrosterOn, lightTypeToString(type));
}

bool CarControlHandler::setLightColor(const std::string& zoneId, aace::carControl::CarControl::LightType type, LightColor color) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());

    std::string lightTypeString = lightTypeToString(type);
    document.AddMember(
        "type",
        rapidjson::Value().SetString(lightTypeString.c_str(), lightTypeString.length()),
        document.GetAllocator());

    std::string lightColorString = lightColorToString(color);
    document.AddMember(
        "color",
        rapidjson::Value().SetString(lightColorString.c_str(), lightColorString.length()),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    responseDispatcher->sendDirective(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_LIGHT_SET_COLOR,
            buffer.GetString());
    return true;
}

bool CarControlHandler::getLightColor(const std::string& zoneId, aace::carControl::CarControl::LightType type, LightColor& color) {
    std::string responseStr;

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());

    std::string lightTypeString = lightTypeToString(type);
    document.AddMember(
        "type",
        rapidjson::Value().SetString(lightTypeString.c_str(), lightTypeString.length()),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    bool callResult = m_callGetLightColor.makeCallAndWaitForResponse(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_LIGHT_GET_COLOR,
            buffer.GetString(), responseStr);
    if (callResult) {
        rapidjson::Document document;
        rapidjson::ParseResult parseResult = document.Parse(responseStr.c_str());
        if (!parseResult) {
            m_logger->log(Level::WARN, TAG, rapidjson::GetParseError_En(parseResult.Code()));
            return false;
        }

        auto root = document.GetObject();
        if (root.HasMember(JSON_ATTR_CARCONTROL_LIGHT_COLOR.c_str()) &&
                root[JSON_ATTR_CARCONTROL_LIGHT_COLOR.c_str()].IsString()) {
            color = lightColorFromString(root[JSON_ATTR_CARCONTROL_LIGHT_COLOR.c_str()].GetString());
        } else {
            m_logger->log(Level::WARN, TAG, "Missing attribute: " +
                    JSON_ATTR_CARCONTROL_LIGHT_COLOR);
            return false;
        }

        return true;
    } else {
        m_logger->log(Level::WARN, TAG, "Failed to obtain response for " + ACTION_CARCONTROL_LIGHT_GET_COLOR);
        return false;
    }
}

// onReceivedEvent
void CarControlHandler::onReceivedEvent(const std::string &action, const std::string &payload) {
    if (action == ACTION_CARCONTROL_CLIMATE_IS_ON_RESPONSE) {
        m_callIsClimateControlOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_CLIMATE_SYNC_IS_ON_RESPONSE) {
        m_callIsClimateControlSyncOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_AIR_RECIRCULATION_IS_ON_RESPONSE) {
        m_callIsAirRecirculationOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_AC_IS_ON_RESPONSE) {
        m_callIsAirConditionerOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_AC_GET_MODE_RESPONSE) {
        m_callGetAirConditionerMode.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_HEATER_IS_ON_RESPONSE) {
        m_callIsHeaterOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_HEATER_GET_TEMPERATURE_RESPONSE) {
        m_callGetHeaterTemperature.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_FAN_IS_ON_RESPONSE) {
        m_callIsFanOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_FAN_GET_SPEED_RESPONSE) {
        m_callGetFanSpeed.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_VENT_IS_ON_RESPONSE) {
        m_callIsVentOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_VENT_GET_POSITION_RESPONSE) {
        m_callGetVentPosition.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_WINDOW_DEFROSTER_IS_ON_RESPONSE) {
        m_callIsWindowDefrosterOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_LIGHT_IS_ON_RESPONSE) {
        m_callIsLightOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_LIGHT_GET_COLOR_RESPONSE) {
        m_callGetLightColor.responseAvailable(payload);
    }
}

// Helper methods
bool CarControlHandler::setValue(const std::string& zoneId, double value, const std::string& action) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());

    document.AddMember(
        "value",
        rapidjson::Value().SetDouble(value),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    responseDispatcher->sendDirective(
            TOPIC_CARCONTROL,
            action,
            buffer.GetString());
    return true;
}

bool CarControlHandler::adjustValue(const std::string& zoneId, double delta, const std::string& action) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());

    document.AddMember(
        "delta",
        rapidjson::Value().SetDouble(delta),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    responseDispatcher->sendDirective(
            TOPIC_CARCONTROL,
            action,
            buffer.GetString());
    return true;
}

bool CarControlHandler::getValue(const std::string& zoneId, double& value,
        const std::string& action, bridge::SyncOverAsync& syncOverAsyncHelper) {
    std::string responseStr;

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    bool callResult = syncOverAsyncHelper.makeCallAndWaitForResponse(
            TOPIC_CARCONTROL,
            action,
            buffer.GetString(), responseStr);
    if (callResult) {
        rapidjson::Document document;
        rapidjson::ParseResult parseResult = document.Parse(responseStr.c_str());
        if (!parseResult) {
            m_logger->log(Level::WARN, TAG, rapidjson::GetParseError_En(parseResult.Code()));
            return false;
        }

        auto root = document.GetObject();
        if (root.HasMember(JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE.c_str()) &&
                root[JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE.c_str()].IsDouble()) {
            value = root[JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE.c_str()].GetDouble();
        } else {
            m_logger->log(Level::WARN, TAG, "Missing attribute: " +
                    JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE);
            return false;
        }

        return true;
    } else {
        m_logger->log(Level::WARN, TAG, "Failed to obtain response for " + action);
        return false;
    }
}

bool CarControlHandler::turnOnOffEndPoint(const std::string& zoneId, const std::string& action, const std::string& type = "") {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());
    
    if (!type.empty()) {
        document.AddMember(
            "type",
            rapidjson::Value().SetString(type.c_str(), type.length()),
            document.GetAllocator());
    }

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    responseDispatcher->sendDirective(
            TOPIC_CARCONTROL,
            action,
            buffer.GetString());
    return true;
}

bool CarControlHandler::isEndPointTurnedOn(const std::string& zoneId, bool& isOn,
        const std::string& action, bridge::SyncOverAsync& syncOverAsyncHelper, const std::string& type = "") {
    std::string responseStr;

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "zoneId",
        rapidjson::Value().SetString(zoneId.c_str(), zoneId.length()),
        document.GetAllocator());

    if (!type.empty()) {
        document.AddMember(
            "type",
            rapidjson::Value().SetString(type.c_str(), type.length()),
            document.GetAllocator());
    }

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    bool callResult = syncOverAsyncHelper.makeCallAndWaitForResponse(
            TOPIC_CARCONTROL,
            action,
            buffer.GetString(), responseStr);
    if (callResult) {
        rapidjson::Document document;
        rapidjson::ParseResult parseResult = document.Parse(responseStr.c_str());
        if (!parseResult) {
            m_logger->log(Level::WARN, TAG, rapidjson::GetParseError_En(parseResult.Code()));
            return false;
        }

        auto root = document.GetObject();
        bool status = false;
        if (root.HasMember(JSON_ATTR_CARCONTROL_ENDPOINT_ON_STATUS.c_str()) &&
                root[JSON_ATTR_CARCONTROL_ENDPOINT_ON_STATUS.c_str()].IsBool()) {
            status = root[JSON_ATTR_CARCONTROL_ENDPOINT_ON_STATUS.c_str()].GetBool();
        } else {
            m_logger->log(Level::WARN, TAG, "Missing attribute: " +
                    JSON_ATTR_CARCONTROL_ENDPOINT_ON_STATUS);
            return false;
        }

        isOn = status;
        return true;
    } else {
        m_logger->log(Level::WARN, TAG, "Failed to obtain response for " + action);
        return false;
    }
}

std::string CarControlHandler::airConditionerModeToString(const aace::carControl::CarControl::AirConditionerMode& mode) {
    std::string airConditionerModeString("UNKNOWN");

    switch (mode) {
        case aace::carControl::CarControl::AirConditionerMode::AUTO:
            airConditionerModeString = VALUE_CARCONTROL_AC_MODE_AUTO;
            break;
        case aace::carControl::CarControl::AirConditionerMode::ECONOMY:
            airConditionerModeString = VALUE_CARCONTROL_AC_MODE_ECONOMY;
            break;
        case aace::carControl::CarControl::AirConditionerMode::MANUAL:
            airConditionerModeString = VALUE_CARCONTROL_AC_MODE_MANUAL;
            break;
        case aace::carControl::CarControl::AirConditionerMode::MAXIMUM:
            airConditionerModeString = VALUE_CARCONTROL_AC_MODE_MAXIMUM;
            break;
        default:
            m_logger->log(Level::ERROR, TAG, "Unknown air conditioner mode received.");
            break;
    }

    return airConditionerModeString;
}

aace::carControl::CarControl::AirConditionerMode CarControlHandler::airConditionerModeFromString(const std::string& modeString) {
    aace::carControl::CarControl::AirConditionerMode airConditionerMode = aace::carControl::CarControl::AirConditionerMode::AUTO;

    if (modeString == VALUE_CARCONTROL_AC_MODE_AUTO) {
        airConditionerMode = aace::carControl::CarControl::AirConditionerMode::AUTO;
    }
    else if (modeString == VALUE_CARCONTROL_AC_MODE_ECONOMY) {
        airConditionerMode = aace::carControl::CarControl::AirConditionerMode::ECONOMY;
    }
    else if (modeString == VALUE_CARCONTROL_AC_MODE_MANUAL) {
        airConditionerMode = aace::carControl::CarControl::AirConditionerMode::MANUAL;
    }
    else if (modeString == VALUE_CARCONTROL_AC_MODE_MAXIMUM) {
        airConditionerMode = aace::carControl::CarControl::AirConditionerMode::MAXIMUM;
    } else {
        m_logger->log(Level::ERROR, TAG, "Unknown air conditioner mode string received.");
    }

    return airConditionerMode;
}

std::string CarControlHandler::lightTypeToString(const aace::carControl::CarControl::LightType& type) {
    std::string lightTypeString("UNKNOWN");

    switch (type) {
        case aace::carControl::CarControl::LightType::AMBIENT_LIGHT:
            lightTypeString = VALUE_CARCONTROL_LIGHT_TYPE_AMBIENT_LIGHT;
            break;
        case aace::carControl::CarControl::LightType::DOME_LIGHT:
            lightTypeString = VALUE_CARCONTROL_LIGHT_TYPE_DOME_LIGHT;
            break;
        case aace::carControl::CarControl::LightType::LIGHT:
            lightTypeString = VALUE_CARCONTROL_LIGHT_TYPE_LIGHT;
            break;
        case aace::carControl::CarControl::LightType::READING_LIGHT:
            lightTypeString = VALUE_CARCONTROL_LIGHT_TYPE_READING_LIGHT;
            break;
        case aace::carControl::CarControl::LightType::TRUNK_LIGHT:
            lightTypeString = VALUE_CARCONTROL_LIGHT_TYPE_TRUNK_LIGHT;
            break;
        default:
            m_logger->log(Level::ERROR, TAG, "Unknown light type received.");
            break;
    }

    return lightTypeString;
}

std::string CarControlHandler::lightColorToString(const aace::carControl::CarControl::LightColor& color) {
    std::string lightColorString("UNKNOWN");

    switch (color) {
        case aace::carControl::CarControl::LightColor::WHITE:
            lightColorString = VALUE_CARCONTROL_LIGHT_COLOR_WHITE;
            break;
        case aace::carControl::CarControl::LightColor::RED:
            lightColorString = VALUE_CARCONTROL_LIGHT_COLOR_RED;
            break;
        case aace::carControl::CarControl::LightColor::ORANGE:
            lightColorString = VALUE_CARCONTROL_LIGHT_COLOR_ORANGE;
            break;
        case aace::carControl::CarControl::LightColor::YELLOW:
            lightColorString = VALUE_CARCONTROL_LIGHT_COLOR_YELLOW;
            break;
        case aace::carControl::CarControl::LightColor::GREEN:
            lightColorString = VALUE_CARCONTROL_LIGHT_COLOR_GREEN;
            break;
        case aace::carControl::CarControl::LightColor::BLUE:
            lightColorString = VALUE_CARCONTROL_LIGHT_COLOR_BLUE;
            break;
        case aace::carControl::CarControl::LightColor::INDIGO:
            lightColorString = VALUE_CARCONTROL_LIGHT_COLOR_INDIGO;
            break;
        case aace::carControl::CarControl::LightColor::VIOLET:
            lightColorString = VALUE_CARCONTROL_LIGHT_COLOR_VIOLET;
            break;
        default:
            m_logger->log(Level::ERROR, TAG, "Unknown light color received.");
            break;
    }

    return lightColorString;
}

aace::carControl::CarControl::LightColor CarControlHandler::lightColorFromString(const std::string& colorString) {
    aace::carControl::CarControl::LightColor lightColor = aace::carControl::CarControl::LightColor::WHITE;

    if (VALUE_CARCONTROL_LIGHT_COLOR_WHITE == colorString) {
        lightColor = aace::carControl::CarControl::LightColor::WHITE;
    } else if (VALUE_CARCONTROL_LIGHT_COLOR_RED == colorString) {
        lightColor = aace::carControl::CarControl::LightColor::RED;
    } else if (VALUE_CARCONTROL_LIGHT_COLOR_ORANGE == colorString) {
        lightColor = aace::carControl::CarControl::LightColor::ORANGE;
    } else if (VALUE_CARCONTROL_LIGHT_COLOR_YELLOW == colorString) {
        lightColor = aace::carControl::CarControl::LightColor::YELLOW;
    } else if (VALUE_CARCONTROL_LIGHT_COLOR_GREEN == colorString) {
        lightColor = aace::carControl::CarControl::LightColor::GREEN;
    } else if (VALUE_CARCONTROL_LIGHT_COLOR_BLUE == colorString) {
        lightColor = aace::carControl::CarControl::LightColor::BLUE;
    } else if (VALUE_CARCONTROL_LIGHT_COLOR_INDIGO == colorString) {
        lightColor = aace::carControl::CarControl::LightColor::INDIGO;
    } else if (VALUE_CARCONTROL_LIGHT_COLOR_VIOLET == colorString) {
        lightColor = aace::carControl::CarControl::LightColor::VIOLET;
    } else {
        m_logger->log(Level::ERROR, TAG, "Unknown colorString received." + colorString);
    }

    return lightColor;
}

std::string CarControlHandler::ventPositionToString(const aace::carControl::CarControl::VentPosition& vent) {
    std::string ventString("UNKNOWN");

    switch (vent) {
        case VentPosition::BODY:
            ventString = VALUE_CARCONTROL_VENT_POSITION_BODY;
            break;
        case VentPosition::MIX:
            ventString = VALUE_CARCONTROL_VENT_POSITION_MIX;
            break;
        case VentPosition::FLOOR:
            ventString = VALUE_CARCONTROL_VENT_POSITION_FLOOR;
            break;
        case VentPosition::WINDSHIELD:
            ventString = VALUE_CARCONTROL_VENT_POSITION_WINDSHIELD;
            break;
        default:
            m_logger->log(Level::ERROR, TAG, "Unknown vent received.");
            break;
    }

    return ventString;
}

aace::carControl::CarControl::VentPosition CarControlHandler::ventPositionFromString(const std::string& ventString) {
    aace::carControl::CarControl::VentPosition vent = aace::carControl::CarControl::VentPosition::WINDSHIELD;

    if (VALUE_CARCONTROL_VENT_POSITION_BODY == ventString) {
        vent = aace::carControl::CarControl::VentPosition::BODY;
    } else if (VALUE_CARCONTROL_VENT_POSITION_MIX == ventString) {
        vent = aace::carControl::CarControl::VentPosition::MIX;
    } else if (VALUE_CARCONTROL_VENT_POSITION_FLOOR == ventString) {
        vent = aace::carControl::CarControl::VentPosition::FLOOR;
    } else if (VALUE_CARCONTROL_VENT_POSITION_WINDSHIELD == ventString) {
        vent = aace::carControl::CarControl::VentPosition::WINDSHIELD;
    } else {
        m_logger->log(Level::ERROR, TAG, "Unknown ventString received.");
    }

    return vent;
}

}  // namespace carControl
}  // namespace aasb