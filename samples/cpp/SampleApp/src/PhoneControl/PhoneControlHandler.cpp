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

#include "SampleApp/PhoneControl/PhoneControlHandler.h"

// C++ Standard Library
#include <regex>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace phoneControl {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PhoneControlHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

PhoneCallControllerHandler::PhoneCallControllerHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler)
    : m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> PhoneCallControllerHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> PhoneCallControllerHandler::getLoggerHandler() { return m_loggerHandler; }

// aace::phoneCallController::PhoneCallController interface

bool PhoneCallControllerHandler::dial(const std::string &payload) {
    log(logger::LoggerHandler::Level::INFO, "dial:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("PCC dial:", payload);
        }
    });
    return true;
}

bool PhoneCallControllerHandler::redial(const std::string &payload) {
    log(logger::LoggerHandler::Level::INFO, "redial:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("PCC redial:", payload);
        }
    });
    return true;
}

void PhoneCallControllerHandler::answer(const std::string &payload) {
    log(logger::LoggerHandler::Level::INFO, "answer:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("PCC answer:", payload);
        }
    });
}

void PhoneCallControllerHandler::stop(const std::string &payload) {
    log(logger::LoggerHandler::Level::INFO, "stop:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("PCC stop:", payload);
        }
    });
}

void PhoneCallControllerHandler::sendDTMF(const std::string &payload) {
    log(logger::LoggerHandler::Level::INFO, "sendDTMF:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("PCC send DTMF:", payload);
        }
    });
}

// private

void PhoneCallControllerHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "PhoneCallControllerHandler", message);
}

void PhoneCallControllerHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // connectionStateChanged
    activity->registerObserver(Event::onPhoneCallControllerConnectionStateChanged, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerConnectionStateChanged:" + value);
        // clang-format off
        static const std::map<std::string, ConnectionState> ConnectionStateEnumerator{
            {"CONNECTED", ConnectionState::CONNECTED},
            {"DISCONNECTED", ConnectionState::DISCONNECTED}
        };
        // clang-format on
        if (ConnectionStateEnumerator.count(value) == 0) {
            return false;
        }
        connectionStateChanged(ConnectionStateEnumerator.at(value));
        return true;
    });

    // callStateChanged
    activity->registerObserver(Event::onPhoneCallControllerCallStateChanged, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerCallStateChanged:" + value);
        static std::regex r("(.+)/(.+)/(.+)", std::regex::optimize);
        std::smatch sm{};
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 3)) {
            return false;
        }
        // clang-format off
        static const std::map<std::string, CallState> CallStateEnumerator{
            {"IDLE", CallState::IDLE},
            {"DIALING", CallState::DIALING},
            {"OUTBOUND_RINGING", CallState::OUTBOUND_RINGING},
            {"ACTIVE", CallState::ACTIVE},
            {"CALL_RECEIVED", CallState::CALL_RECEIVED},
            {"INBOUND_RINGING", CallState::INBOUND_RINGING}
        };
        // clang-format on
        if (CallStateEnumerator.count(sm[1]) == 0) {
            return false;
        }
        callStateChanged(CallStateEnumerator.at(sm[1]), sm[2], sm[3]);
        return true;
    });

    // callFailed
    activity->registerObserver(Event::onPhoneCallControllerCallFailed, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerCallFailed:" + value);
        static std::regex r("(.+)/(.+)/(.+)", std::regex::optimize);
        std::smatch sm{};
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 3)) {
            return false;
        }
        // clang-format off
        static const std::map<std::string, CallError> CallErrorEnumerator{
            {"NO_CARRIER", CallError::NO_CARRIER},
            {"BUSY", CallError::BUSY},
            {"NO_ANSWER", CallError::NO_ANSWER},
            {"NO_NUMBER_FOR_REDIAL", CallError::NO_NUMBER_FOR_REDIAL},
            {"OTHER", CallError::OTHER}
        };
        // clang-format on
        if (CallErrorEnumerator.count(sm[2]) == 0) {
            return false;
        }
        callFailed(sm[1], CallErrorEnumerator.at(sm[2]), sm[3]);
        return true;
    });

    // callerIdReceived
    activity->registerObserver(Event::onPhoneCallControllerCallerIdReceived, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerCallerIdReceived:" + value);
        static std::regex r("(.+)/(.+)", std::regex::optimize);
        std::smatch sm{};
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 2)) {
            return false;
        }
        callerIdReceived(sm[1], sm[2]);
        return true;
    });

    // sendDTMFSucceeded
    activity->registerObserver(Event::onPhoneCallControllerSendDTMFSucceeded, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerSendDTMFSucceeded:" + value);
        sendDTMFSucceeded(value);
        return true;
    });

    // sendDTMFFailed
    activity->registerObserver(Event::onPhoneCallControllerSendDTMFFailed, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerSendDTMFFailed:" + value);
        static std::regex r("(.+)/(.+)/(.+)", std::regex::optimize);
        std::smatch sm{};
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 3)) {
            return false;
        }
        // clang-format off
        static const std::map<std::string, DTMFError> DTMFErrorEnumerator{
            {"CALL_NOT_IN_PROGRESS", DTMFError::CALL_NOT_IN_PROGRESS},
            {"DTMF_FAILED", DTMFError::DTMF_FAILED}
        };
        // clang-format on
        if (DTMFErrorEnumerator.count(sm[2]) == 0) {
            return false;
        }
        sendDTMFFailed(sm[1], DTMFErrorEnumerator.at(sm[2]), sm[3]);
        return true;
    });
}

} // namespace phoneControl
} // namespace sampleApp
