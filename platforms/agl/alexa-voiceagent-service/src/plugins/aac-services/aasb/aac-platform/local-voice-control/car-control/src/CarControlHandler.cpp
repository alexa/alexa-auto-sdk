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
        m_callIsPowerControllerOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callIsToggleControllerOn(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callGetModeControllerValue(logger, responseDispatcher, CALL_WAIT_TIMEOUT),
        m_callGetRangeControllerValue(logger, responseDispatcher, CALL_WAIT_TIMEOUT) {
}

bool CarControlHandler::turnPowerControllerOn(const std::string& controlId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(controlId, ACTION_CARCONTROL_TURN_POWER_CONTROLLER_ON);
} 

bool CarControlHandler::turnPowerControllerOff(const std::string& controlId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(controlId, ACTION_CARCONTROL_TURN_POWER_CONTROLLER_OFF);
}

bool CarControlHandler::isPowerControllerOn(const std::string& controlId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(controlId, isOn, ACTION_CARCONTROL_IS_POWER_CONTROLLER_ON, m_callIsPowerControllerOn);
} 

bool CarControlHandler::turnToggleControllerOn(const std::string& controlId, const std::string& controllerId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(controlId, ACTION_CARCONTROL_TURN_TOGGLE_CONTROLLER_ON, controllerId);
} 

bool CarControlHandler::turnToggleControllerOff(const std::string& controlId, const std::string& controllerId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return turnOnOffEndPoint(controlId, ACTION_CARCONTROL_TURN_TOGGLE_CONTROLLER_OFF, controllerId);
}

bool CarControlHandler::isToggleControllerOn(const std::string& controlId, const std::string& controllerId, bool& isOn) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return isEndPointTurnedOn(controlId, isOn, ACTION_CARCONTROL_IS_TOGGLE_CONTROLLER_ON, m_callIsToggleControllerOn, controllerId);
} 

bool CarControlHandler::setRangeControllerValue(const std::string& controlId, const std::string& controllerId, double value) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "controlId",
        rapidjson::Value().SetString(controlId.c_str(), controlId.length()),
        document.GetAllocator());

    document.AddMember(
        "controllerId",
        rapidjson::Value().SetString(controllerId.c_str(), controllerId.length()),
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
            ACTION_CARCONTROL_SET_RANGE_CONTROLLER_VALUE,
            buffer.GetString());

    return true;
}

bool CarControlHandler::adjustRangeControllerValue(const std::string& controlId, const std::string& controllerId, double delta) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "controlId",
        rapidjson::Value().SetString(controlId.c_str(), controlId.length()),
        document.GetAllocator());

    document.AddMember(
        "controllerId",
        rapidjson::Value().SetString(controllerId.c_str(), controllerId.length()),
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
            ACTION_CARCONTROL_ADJUST_RANGE_CONTROLLER_VALUE,
            buffer.GetString());

    return true;

}

bool CarControlHandler::getRangeControllerValue(const std::string& controlId, const std::string& controllerId, double& value) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    std::string responseStr;

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "controlId",
        rapidjson::Value().SetString(controlId.c_str(), controlId.length()),
        document.GetAllocator());

    document.AddMember(
        "controllerId",
        rapidjson::Value().SetString(controllerId.c_str(), controllerId.length()),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    bool callResult = m_callGetRangeControllerValue.makeCallAndWaitForResponse(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_GET_RANGE_CONTROLLER_VALUE,
            buffer.GetString(), responseStr);

    if (callResult) {
        rapidjson::Document document;
        rapidjson::ParseResult parseResult = document.Parse(responseStr.c_str());

        if (!parseResult) {
            m_logger->log(Level::WARN, TAG, rapidjson::GetParseError_En(parseResult.Code()));
            return false;
        }

        auto root = document.GetObject();

        if (root.HasMember(JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE.c_str())) {
            if (root[JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE.c_str()].IsNumber()) {
                value = root[JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE.c_str()].GetDouble();
            } else {
                m_logger->log(Level::WARN, TAG, "Invalid value for: " +
                        JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE);
                return false;
            }
        } else {
            m_logger->log(Level::WARN, TAG, "Missing attribute: " +
                    JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE);
            return false;
        }

        return true;
    } else {
        m_logger->log(Level::WARN, TAG, "Failed to obtain response for " + ACTION_CARCONTROL_GET_RANGE_CONTROLLER_VALUE);
        return false;
    }
}

bool CarControlHandler::setModeControllerValue(const std::string& controlId, const std::string& controllerId, const std::string& value) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "controlId",
        rapidjson::Value().SetString(controlId.c_str(), controlId.length()),
        document.GetAllocator());

    document.AddMember(
        "controllerId",
        rapidjson::Value().SetString(controllerId.c_str(), controllerId.length()),
        document.GetAllocator());

    document.AddMember(
        "value",
        rapidjson::Value().SetString(value.c_str(), value.length()),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    responseDispatcher->sendDirective(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_SET_MODE_CONTROLLER_VALUE,
            buffer.GetString());

    return true;
}

bool CarControlHandler::adjustModeControllerValue(const std::string& controlId, const std::string& controllerId, int delta) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "controlId",
        rapidjson::Value().SetString(controlId.c_str(), controlId.length()),
        document.GetAllocator());

    document.AddMember(
        "controllerId",
        rapidjson::Value().SetString(controllerId.c_str(), controllerId.length()),
        document.GetAllocator());

    document.AddMember(
        "delta",
        rapidjson::Value().SetInt(delta),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    responseDispatcher->sendDirective(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_ADJUST_MODE_CONTROLLER_VALUE,
            buffer.GetString());

    return true;
}

bool CarControlHandler::getModeControllerValue(const std::string& controlId, const std::string& controllerId, std::string& value) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    std::string responseStr;

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "controlId",
        rapidjson::Value().SetString(controlId.c_str(), controlId.length()),
        document.GetAllocator());

    document.AddMember(
        "controllerId",
        rapidjson::Value().SetString(controllerId.c_str(), controllerId.length()),
        document.GetAllocator());

    // create payload string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    bool callResult = m_callGetModeControllerValue.makeCallAndWaitForResponse(
            TOPIC_CARCONTROL,
            ACTION_CARCONTROL_GET_MODE_CONTROLLER_VALUE,
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
                root[JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE.c_str()].IsString()) {
            value = root[JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE.c_str()].GetString();
        } else {
            m_logger->log(Level::WARN, TAG, "Missing attribute: " +
                    JSON_ATTR_CARCONTROL_ENDPOINT_ACTUAL_VALUE);
            return false;
        }

        return true;
    } else {
        m_logger->log(Level::WARN, TAG, "Failed to obtain response for " + ACTION_CARCONTROL_GET_MODE_CONTROLLER_VALUE);
        return false;
    }
}

// onReceivedEvent
void CarControlHandler::onReceivedEvent(const std::string &action, const std::string &payload) {
    if (action == ACTION_CARCONTROL_IS_POWER_CONTROLLER_ON_RESPONSE) {
        m_callIsPowerControllerOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_IS_TOGGLE_CONTROLLER_ON_RESPONSE) {
        m_callIsToggleControllerOn.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_GET_MODE_CONTROLLER_VALUE_RESPONSE) {
        m_callGetModeControllerValue.responseAvailable(payload);
    } else if (action == ACTION_CARCONTROL_GET_RANGE_CONTROLLER_VALUE_RESPONSE) {
        m_callGetRangeControllerValue.responseAvailable(payload);
    }
}

bool CarControlHandler::turnOnOffEndPoint(const std::string& controlId, const std::string& action, const std::string& controllerId) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "controlId",
        rapidjson::Value().SetString(controlId.c_str(), controlId.length()),
        document.GetAllocator());
    
    if (!controllerId.empty()) {
        document.AddMember(
            "controllerId",
            rapidjson::Value().SetString(controllerId.c_str(), controllerId.length()),
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

bool CarControlHandler::isEndPointTurnedOn(const std::string& controlId, bool& isOn,
        const std::string& action, bridge::SyncOverAsync& syncOverAsyncHelper, const std::string& controllerId) {
    std::string responseStr;

    rapidjson::Document document;
    document.SetObject();
    document.AddMember(
        "controlId",
        rapidjson::Value().SetString(controlId.c_str(), controlId.length()),
        document.GetAllocator());

    if (!controllerId.empty()) {
        document.AddMember(
            "controllerId",
            rapidjson::Value().SetString(controllerId.c_str(), controllerId.length()),
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

}  // namespace carControl
}  // namespace aasb