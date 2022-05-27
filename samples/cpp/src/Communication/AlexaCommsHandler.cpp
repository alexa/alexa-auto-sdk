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

#include "SampleApp/Communication/AlexaCommsHandler.h"

#include <AASB/Message/Communication/AlexaComms/AcceptCallMessage.h>
#include <AASB/Message/Communication/AlexaComms/CallDisplayInfoMessage.h>
#include <AASB/Message/Communication/AlexaComms/CallStateChangedMessage.h>
#include <AASB/Message/Communication/AlexaComms/StopCallMessage.h>

// C++ Standard Library
#include <regex>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace communication {

REGISTER_EXTENSION("AlexaComms", AlexaCommsHandler);

using namespace aasb::message::communication::alexaComms;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaCommsHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool AlexaCommsHandler::initialize(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) {
    log(logger::LoggerHandler::Level::INFO, "Initialize AlexaCommsHandler");
    m_activity = activity;
    m_loggerHandler = loggerHandler;
    m_callDisplayInfo = "";
    m_callState = CallState::NONE;
    m_messageBroker = messageBroker;
    setupUI();
    subscribeToAASBMessages();
    return true;
}

bool AlexaCommsHandler::validate(const std::vector<nlohmann::json>& configs) {
    // Look for comms config
    for (auto const& j : configs) {
        try {
            auto obj = j.at("aace.alexa").at("avsDeviceSDK").at("communications");
            if (obj.is_object()) {
                return true;
            }
        } catch (nlohmann::json::exception& e) {
        }
    }

    return false;
}

std::weak_ptr<Activity> AlexaCommsHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AlexaCommsHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void AlexaCommsHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to CallDisplayInfo Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleCallDisplayInfoMessage(message); },
        CallDisplayInfoMessage::topic(),
        CallDisplayInfoMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to CallStateChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleCallStateChangedMessage(message); },
        CallStateChangedMessage::topic(),
        CallStateChangedMessage::action());
}

void AlexaCommsHandler::handleCallDisplayInfoMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received CallDisplayInfoMessage");
    CallDisplayInfoMessage msg = json::parse(message);
    callDisplayInfo(msg.payload.displayInfo);
}

void AlexaCommsHandler::handleCallStateChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received CallStateChangedMessage");
    CallStateChangedMessage msg = json::parse(message);
    callStateChanged(msg.payload.state);
}

void AlexaCommsHandler::acceptCall() {
    AcceptCallMessage msg;
    m_messageBroker->publish(msg.toString());
}

void AlexaCommsHandler::stopCall() {
    StopCallMessage msg;
    m_messageBroker->publish(msg.toString());
}

void AlexaCommsHandler::callDisplayInfo(const std::string& displayInfo) {
    log(logger::LoggerHandler::Level::VERBOSE, "callDisplayInfo:" + displayInfo);
    // Update call display info
    if (auto activity = m_activity.lock()) {
        activity->runOnUIThread([=]() {
            if (auto card = activity->findViewById("id:card").lock()) {
                card->set(displayInfo, View::Type::CommunicationCallDisplayInfo);
            }
        });
    }
    m_callDisplayInfo = displayInfo;
}

void AlexaCommsHandler::callStateChanged(CallState state) {
    log(logger::LoggerHandler::Level::VERBOSE, "callStateChanged:" + callStateToString(state));

    // Update call state
    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine(
            "Communication call state changed from " + callStateToString(m_callState) + " to " +
            callStateToString(state));
        console->printRuler();
    }
    m_callState = state;
}

void AlexaCommsHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // onCommunicationAcceptCall
    activity->registerObserver(Event::onCommunicationAcceptCall, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onCommunicationAcceptCall");

        if (m_callState == CallState::INBOUND_RINGING) {
            acceptCall();
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Communication call accepted");
                console->printRuler();
            }
        } else if (m_callState == CallState::CALL_CONNECTED) {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Communication call is already connected");
                console->printRuler();
            }
        } else {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("There is no current call to accept");
                console->printRuler();
            }
        }

        return true;
    });

    // onCommunicationStopCall
    activity->registerObserver(Event::onCommunicationStopCall, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onCommunicationStopCall");

        stopCall();
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Communication call stopped");
            console->printRuler();
        }

        return true;
    });

    // onCommunicationShowDisplayInfo
    activity->registerObserver(Event::onCommunicationShowDisplayInfo, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onCommunicationShowDisplayInfo");
        showDisplayInfo();
        return true;
    });

    // onCommunicationShowState
    activity->registerObserver(Event::onCommunicationShowState, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onCommunicationShowState");
        showState();
        return true;
    });
}

void AlexaCommsHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AlexaCommsHandler", message);
}

void AlexaCommsHandler::showDisplayInfo() {
    log(logger::LoggerHandler::Level::VERBOSE, "Showing communication display info");
    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine("Communication call display info: " + m_callDisplayInfo);
        console->printRuler();
    }
}

void AlexaCommsHandler::showState() {
    log(logger::LoggerHandler::Level::VERBOSE, "Showing communication state");
    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine("Communication call state: " + callStateToString(m_callState));
        console->printRuler();
    }
}

std::string AlexaCommsHandler::callStateToString(CallState state) {
    static const std::map<CallState, std::string> callStateMap{{CallState::CONNECTING, "CONNECTING"},
                                                               {CallState::INBOUND_RINGING, "INBOUND_RINGING"},
                                                               {CallState::CALL_CONNECTED, "CALL_CONNECTED"},
                                                               {CallState::CALL_DISCONNECTED, "CALL_DISCONNECTED"},
                                                               {CallState::NONE, "NONE"}};

    return callStateMap.at(state);
}

}  // namespace communication
}  // namespace sampleApp
