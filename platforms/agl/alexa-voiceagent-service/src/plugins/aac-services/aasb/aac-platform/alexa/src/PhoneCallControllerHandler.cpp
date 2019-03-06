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

#include "PhoneCallControllerHandler.h"

#include <rapidjson/document.h>

#include <aasb/Consts.h>

#include <sstream>

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

// Namespaces.
using namespace aasb::bridge;

// Logging tag for this file.
const std::string TAG = "aasb::phoneCallController::PhoneCallControllerHandler";

namespace aasb {
namespace phoneCallController {

std::shared_ptr<PhoneCallControllerHandler> PhoneCallControllerHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) {
    auto phoneCallControllerHandler =
        std::shared_ptr<PhoneCallControllerHandler>(new PhoneCallControllerHandler(logger, directiveDispatcher));

    return phoneCallControllerHandler;
}

PhoneCallControllerHandler::PhoneCallControllerHandler(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) :
        m_logger(logger),
        m_directiveDispatcher(directiveDispatcher) {

}

bool PhoneCallControllerHandler::dial(const std::string &payload) {
    m_logger->log(Level::VERBOSE, TAG, "dial");

    auto directiveDispatcher = m_directiveDispatcher.lock();
    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "dial: Directive dispatcher is out of scope");
        return false;
    }

    directiveDispatcher->sendDirective(TOPIC_PHONECALL_CONTROLLER, ACTION_PHONECALL_DIAL, payload);
    return true;
}

bool PhoneCallControllerHandler::redial(const std::string &payload) {
    m_logger->log(Level::VERBOSE, TAG, "redial");

    auto directiveDispatcher = m_directiveDispatcher.lock();
    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "redial: Directive dispatcher is out of scope");
        return false;
    }

    directiveDispatcher->sendDirective(TOPIC_PHONECALL_CONTROLLER, ACTION_PHONECALL_REDIAL, payload);
    return true;
}

void PhoneCallControllerHandler::answer(const std::string &payload) {
    m_logger->log(Level::VERBOSE, TAG, "answer");

    auto directiveDispatcher = m_directiveDispatcher.lock();
    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "answer: Directive dispatcher is out of scope");
        return;
    }

    directiveDispatcher->sendDirective(TOPIC_PHONECALL_CONTROLLER, ACTION_PHONECALL_ANSWER, payload);
}

void PhoneCallControllerHandler::stop(const std::string &payload) {
    m_logger->log(Level::VERBOSE, TAG, "stop");

    auto directiveDispatcher = m_directiveDispatcher.lock();
    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "stop: Directive dispatcher is out of scope");
        return;
    }

    directiveDispatcher->sendDirective(TOPIC_PHONECALL_CONTROLLER, ACTION_PHONECALL_STOP, payload);
}

void PhoneCallControllerHandler::sendDTMF(const std::string &payload) {
    m_logger->log(Level::VERBOSE, TAG, "sendDTMF");

    auto directiveDispatcher = m_directiveDispatcher.lock();
    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "sendDTMF: Directive dispatcher is out of scope");
        return;
    }

    directiveDispatcher->sendDirective(TOPIC_PHONECALL_CONTROLLER, ACTION_PHONECALL_SEND_DTMF, payload);
}

void PhoneCallControllerHandler::onReceivedEvent(const std::string& action, const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "processing action: " + action);

    if (action == ACTION_PHONECALL_CONNECTION_STATE_CHANGED) {
        connectionStateChanged(payload);
    } else if (action == ACTION_PHONECALL_CALL_STATE_CHANGED) {
        callStateChanged(payload);
    } else if (action == ACTION_PHONECALL_CALL_FAILED) {
        callFailed(payload);
    } else if (action == ACTION_PHONECALL_CALLER_ID_RECEIVED) {
        callerIdReceived(payload);
    } else if (action == ACTION_PHONECALL_SEND_DTMF_SUCCEEDED) {
        sendDTMFSucceeded(payload);
    } else if (action == ACTION_PHONECALL_SEND_DTMF_FAILED) {
        sendDTMFFailed(payload);
    } else if (action == ACTION_PHONECALL_DEVICE_CONFIGURATION_UPDATED) {
        deviceConfigurationUpdated(payload);
    } else {
        m_logger->log(Level::WARN, TAG, "Unknown action: " + action);
    }
}

void PhoneCallControllerHandler::connectionStateChanged(const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "connectionState payload " + payload);

    rapidjson::Document document;
    document.Parse(payload.c_str());
    auto root = document.GetObject();

    ConnectionState state;
    if (root.HasMember(JSON_ATTR_CONNECTION_STATE.c_str()) && root[JSON_ATTR_CONNECTION_STATE.c_str()].IsString()) {
        std::string connectionStateStr = root[JSON_ATTR_CONNECTION_STATE.c_str()].GetString();
        if (connectionStateStr == VALUE_CONNECTED) {
            state = ConnectionState::CONNECTED;
        } else if (connectionStateStr == VALUE_DISCONNECTED) {
            state = ConnectionState::DISCONNECTED;
        } else {
            m_logger->log(Level::WARN, TAG, "connectionStateChanged: Invalid state " + connectionStateStr);
            return;
        }
    } else {
        m_logger->log(Level::WARN, TAG, "connectionStateChanged: Invalid args " + payload);
        return;
    }

    PhoneCallController::connectionStateChanged(state);
}

void PhoneCallControllerHandler::callStateChanged(const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "callStateChanged payload " + payload);

    rapidjson::Document document;
    document.Parse(payload.c_str());
    auto root = document.GetObject();

    CallState state;
    std::string callId, callerId;

    if (root.HasMember(JSON_ATTR_CALL_STATE.c_str()) && root[JSON_ATTR_CALL_STATE.c_str()].IsString()) {
        std::string callStateStr = root[JSON_ATTR_CALL_STATE.c_str()].GetString();

        if (callStateStr == VALUE_ACTIVE) {
            state = CallState::ACTIVE;
        } else if (callStateStr == VALUE_CALL_RECEIVED) {
            state = CallState::CALL_RECEIVED;
        } else if (callStateStr == VALUE_DIALING) {
            state = CallState::DIALING;
        } else if (callStateStr == VALUE_IDLE) {
            state = CallState::IDLE;
        } else if (callStateStr == VALUE_INBOUND_RINGING) {
            state = CallState::INBOUND_RINGING;
        } else if (callStateStr == VALUE_OUTBOUND_RINGING) {
            state = CallState::OUTBOUND_RINGING;
        } else {
            m_logger->log(Level::WARN, TAG, "callStateChange: Invalid call state " + callStateStr);
            return;
        }
    } else {
        m_logger->log(Level::WARN, TAG, "callStateChange: call state not found");
        return;
    }

    if (root.HasMember(JSON_ATTR_CALL_ID.c_str()) && root[JSON_ATTR_CALL_ID.c_str()].IsString()) {
        callId = root[JSON_ATTR_CALL_ID.c_str()].GetString();
    } else {
        m_logger->log(Level::WARN, TAG, "callStateChange: callId not found");
        return;
    }

    // Optional caller id
    if (root.HasMember(JSON_ATTR_CALLER_ID.c_str()) && root[JSON_ATTR_CALLER_ID.c_str()].IsString()) {
        callerId = root[JSON_ATTR_CALLER_ID.c_str()].GetString();
    }

    PhoneCallController::callStateChanged(state, callId, callerId);
}

void PhoneCallControllerHandler::callFailed(const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "callFailed payload " + payload);

    rapidjson::Document document;
    document.Parse(payload.c_str());
    auto root = document.GetObject();

    CallError error = CallError::OTHER;
    std::string callId, message;

    if (root.HasMember(JSON_ATTR_CALL_ERROR.c_str()) && root[JSON_ATTR_CALL_ERROR.c_str()].IsString()) {
        std::string errorStr = root[JSON_ATTR_CALL_ERROR.c_str()].GetString();

        if (errorStr == VALUE_CALL_ERROR_NO_CARRIER) {
            error = CallError::NO_CARRIER;
        } else if (errorStr == VALUE_CALL_ERROR_BUSY) {
            error = CallError::BUSY;
        } else if (errorStr == VALUE_CALL_ERROR_NO_ANSWER) {
            error = CallError::NO_ANSWER;
        } else if (errorStr == VALUE_CALL_ERROR_NO_NUMBER_FOR_REDIAL) {
            error = CallError::NO_NUMBER_FOR_REDIAL;
        } else if (errorStr == VALUE_CALL_ERROR_OTHER) {
            error = CallError::OTHER;
        } else {
            // Don't fail the entire method send generic error instead.
            m_logger->log(Level::WARN, TAG, "callFailed: Invalid call error " + errorStr);
            error = CallError::OTHER;
        }
    } else {
        m_logger->log(Level::WARN, TAG, "callFailed: call error not found");
        return;
    }

    if (root.HasMember(JSON_ATTR_CALL_ID.c_str()) && root[JSON_ATTR_CALL_ID.c_str()].IsString()) {
        callId = root[JSON_ATTR_CALL_ID.c_str()].GetString();
    } else {
        m_logger->log(Level::WARN, TAG, "callFailed: callId not found");
        return;
    }

    // Optional call error msg
    if (root.HasMember(JSON_ATTR_CALL_ERROR_MSG.c_str()) && root[JSON_ATTR_CALL_ERROR_MSG.c_str()].IsString()) {
        message = root[JSON_ATTR_CALL_ERROR_MSG.c_str()].GetString();
    }

    PhoneCallController::callFailed(callId, error, message);
}

void PhoneCallControllerHandler::callerIdReceived(const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "callerIdReceived payload " + payload);

    rapidjson::Document document;
    document.Parse(payload.c_str());
    auto root = document.GetObject();

    std::string callId, callerId;

    if (root.HasMember(JSON_ATTR_CALL_ID.c_str()) && root[JSON_ATTR_CALL_ID.c_str()].IsString()) {
        callId = root[JSON_ATTR_CALL_ID.c_str()].GetString();
    } else {
        m_logger->log(Level::WARN, TAG, "callIdReceived: callId not found");
        return;
    }

    // Optional caller id
    if (root.HasMember(JSON_ATTR_CALLER_ID.c_str()) && root[JSON_ATTR_CALLER_ID.c_str()].IsString()) {
        callerId = root[JSON_ATTR_CALLER_ID.c_str()].GetString();
    } else {
        m_logger->log(Level::WARN, TAG, "callIdReceived: callerId not found");
        return;
    }

    PhoneCallController::callerIdReceived(callId, callerId);
}

void PhoneCallControllerHandler::sendDTMFSucceeded(const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "sendDTMFSucceeded payload " + payload);

    rapidjson::Document document;
    document.Parse(payload.c_str());
    auto root = document.GetObject();

    std::string callId;

    if (root.HasMember(JSON_ATTR_CALL_ID.c_str()) && root[JSON_ATTR_CALL_ID.c_str()].IsString()) {
        callId = root[JSON_ATTR_CALL_ID.c_str()].GetString();
    } else {
        m_logger->log(Level::WARN, TAG, "sendDTMFSucceeded: callId not found");
        return;
    }

    PhoneCallController::sendDTMFSucceeded(callId);
}

void PhoneCallControllerHandler::sendDTMFFailed(const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "sendDTMFFailed payload " + payload);

    rapidjson::Document document;
    document.Parse(payload.c_str());
    auto root = document.GetObject();

    DTMFError error;
    std::string callId, message;

    if (root.HasMember(JSON_ATTR_DTMF_ERROR.c_str()) && root[JSON_ATTR_DTMF_ERROR.c_str()].IsString()) {
        std::string errorStr = root[JSON_ATTR_DTMF_ERROR.c_str()].GetString();

        if (errorStr == VALUE_DTMF_ERROR_CALL_NOT_IN_PROGRESS) {
            error = DTMFError::CALL_NOT_IN_PROGRESS;
        } else if (errorStr == VALUE_DTMF_ERROR_DTMF_FAILED) {
            error = DTMFError::DTMF_FAILED;
        } else {
            m_logger->log(Level::WARN, TAG, "sendDTMFFailed: Invalid dtmf error " + errorStr);
            return;
        }
    } else {
        m_logger->log(Level::WARN, TAG, "sendDTMFFailed: dtmf error not found");
        return;
    }

    if (root.HasMember(JSON_ATTR_CALL_ID.c_str()) && root[JSON_ATTR_CALL_ID.c_str()].IsString()) {
        callId = root[JSON_ATTR_CALL_ID.c_str()].GetString();
    } else {
        m_logger->log(Level::WARN, TAG, "sendDTMFFailed: callId not found");
        return;
    }

    // Optional dtmf error msg
    if (root.HasMember(JSON_ATTR_DTMF_ERROR_MSG.c_str()) && root[JSON_ATTR_DTMF_ERROR_MSG.c_str()].IsString()) {
        message = root[JSON_ATTR_DTMF_ERROR_MSG.c_str()].GetString();
    }

    PhoneCallController::sendDTMFFailed(callId, error, message);
}

void PhoneCallControllerHandler::deviceConfigurationUpdated(const std::string& payload) {
    // TODO: Implement device configuration updated
}

}  // phoneCallController
}  // aasb