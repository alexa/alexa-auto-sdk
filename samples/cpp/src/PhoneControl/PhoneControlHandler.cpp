/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Core/EngineMacros.h"
#include "SampleApp/PhoneControl/PhoneControlHandler.h"

#include <AASB/Message/PhoneCallController/PhoneCallController/AnswerMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallerIdReceivedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallFailedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallStateChangedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/ConnectionStateChangedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CreateCallIdMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/DeviceConfigurationUpdatedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/DialMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/RedialMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/SendDTMFMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/SendDTMFFailedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/SendDTMFSucceededMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/StopMessage.h>

// C++ Standard Library
#include <regex>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace phoneControl {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PhoneControlHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

static const uint32_t ASYNC_REPLY_TIMEOUT = 1000;

using MessageBroker = aace::core::MessageBroker;

PhoneCallControllerHandler::PhoneCallControllerHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)},
        m_callState{CallState::IDLE},
        m_callError{} {
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> PhoneCallControllerHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> PhoneCallControllerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void PhoneCallControllerHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to Answer Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAnswerMessage(message); },
        AnswerMessage::topic(),
        AnswerMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to Dial Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleDialMessage(message); }, DialMessage::topic(), DialMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to Redial Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleRedialMessage(message); },
        RedialMessage::topic(),
        RedialMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to SendDTMF Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSendDMTFMessage(message); },
        SendDTMFMessage::topic(),
        SendDTMFMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to Stop Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleStopMessage(message); }, StopMessage::topic(), StopMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to CreateCallIdReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleCreateCallIdReplyMessage(message); },
        CreateCallIdMessageReply::topic(),
        CreateCallIdMessageReply::action());
}

void PhoneCallControllerHandler::handleAnswerMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AnswerMessage");
    AnswerMessage msg = json::parse(message);
    answer(msg.payload.payload);
}

void PhoneCallControllerHandler::handleDialMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received DialMessage");
    AnswerMessage msg = json::parse(message);
    dial(msg.payload.payload);
}

void PhoneCallControllerHandler::handleRedialMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received RedialMessage");
    AnswerMessage msg = json::parse(message);
    redial(msg.payload.payload);
}

void PhoneCallControllerHandler::handleSendDMTFMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received SendDMTFMessage");
    SendDTMFMessage msg = json::parse(message);
    sendDTMF(msg.payload.payload);
}

void PhoneCallControllerHandler::handleStopMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received StopMessage");
    StopMessage msg = json::parse(message);
    stop(msg.payload.payload);
}

void PhoneCallControllerHandler::handleCreateCallIdReplyMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received CreateCallIdReplyMessage");
    CreateCallIdMessageReply msg = json::parse(message);

    auto promise = getReplyMessagePromise(msg.header.messageDescription.replyToId);
    ThrowIfNull(promise, "invalidPromise");
    promise->set_value(msg.payload.callId);
}

void PhoneCallControllerHandler::callFailed(const std::string& callId, CallError code, const std::string& message) {
    // Publish the "CallFailed" message
    CallFailedMessage msg;
    msg.payload.callId = callId;
    msg.payload.code = code;
    msg.payload.message = message;
    m_messageBroker->publish(msg.toString());
}

void PhoneCallControllerHandler::callStateChanged(
    CallState state,
    const std::string& callId,
    const std::string& callerId) {
    // Publish the "CallStateChanged" message
    CallStateChangedMessage msg;
    msg.payload.state = state;
    msg.payload.callId = callId;
    msg.payload.callerId = callerId;
    m_messageBroker->publish(msg.toString());
}

void PhoneCallControllerHandler::callerIdReceived(const std::string& callId, const std::string& callerId) {
    // Publish the "CallerIdReceived" message
    CallerIdReceivedMessage msg;
    msg.payload.callId = callId;
    msg.payload.callerId = callerId;
    m_messageBroker->publish(msg.toString());
}

void PhoneCallControllerHandler::connectionStateChanged(ConnectionState state) {
    // Publish the "ConnectionStateChanged" message
    ConnectionStateChangedMessage msg;
    msg.payload.state = state;
    m_messageBroker->publish(msg.toString());
}

std::string PhoneCallControllerHandler::createCallId() {
    // Publish the "CreateCallId" message
    CreateCallIdMessage msg;
    m_messageBroker->publish(msg.toString());
    return waitForAsyncReply(msg.header.id);
}

void PhoneCallControllerHandler::deviceConfigurationUpdated(
    std::unordered_map<CallingDeviceConfigurationProperty, bool> configurationMap) {
    json configuration;
    for (auto it : configurationMap) {
        configuration[configurationFeatureToString(it.first)] = it.second;
    }

    // Publish the "DeviceConfigurationUpdated" message
    DeviceConfigurationUpdatedMessage msg;
    msg.payload.configurationMap = configuration.dump();
    m_messageBroker->publish(msg.toString());
}

void PhoneCallControllerHandler::sendDTMFSucceeded(const std::string& callId) {
    // Publish the "SendDTMFSucceeded" message
    SendDTMFSucceededMessage msg;
    msg.payload.callId = callId;
    m_messageBroker->publish(msg.toString());
}

void PhoneCallControllerHandler::sendDTMFFailed(const std::string& callId, DTMFError code, const std::string& message) {
    // Publish the "SendDTMFFailed" message
    SendDTMFFailedMessage msg;
    msg.payload.callId = callId;
    msg.payload.code = code;
    msg.payload.message = message;
    m_messageBroker->publish(msg.toString());
}

bool PhoneCallControllerHandler::dial(const std::string& payload) {
    log(logger::LoggerHandler::Level::INFO, "dial:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }

    // Save payload of current call
    updatePayload(payload);

    // Assume that we dialed successfully
    m_callState = CallState::ACTIVE;
    m_callError = "";

    activity->runOnUIThread([=]() {
        auto callId = getCallId(m_currentCall);
        callStateChanged(m_callState, callId);
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Phone number to dial: ", getPhoneNumber(m_currentCall));
            console->printLine("Phone call state:     ", callStateToString());
            console->printLine("Phone call id:        ", callId);
            console->printRuler();
        }
    });

    // Current call is last outgoing call
    m_outgoingCall = m_currentCall;

    return true;
}

bool PhoneCallControllerHandler::redial(const std::string& payload) {
    log(logger::LoggerHandler::Level::INFO, "redial:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }

    activity->runOnUIThread([=]() {
        auto redial = getPhoneNumber(m_outgoingCall);
        if (auto console = m_console.lock()) {
            if (!redial.empty()) {
                // Redial only sends back new call id, so update current payload with new call id
                updateCallId(payload);
                // Assume that we dialed successfully
                m_callState = CallState::ACTIVE;
                m_callError = "";
                auto callId = getCallId(m_outgoingCall);
                console->printRuler();
                console->printLine("Phone number to redial: ", redial);
                console->printLine("Phone call state:       ", callStateToString());
                console->printLine("Phone call id:          ", callId);
                console->printRuler();
                callStateChanged(m_callState, callId);
                m_currentCall = m_outgoingCall;
            } else {
                console->printRuler();
                console->printLine("There is no previous outgoing phone number to redial");
                try {
                    json redialPayload = json::parse(payload);
                    auto callId = redialPayload["callId"];
                    callFailed(callId, CallError::NO_NUMBER_FOR_REDIAL);
                } catch (std::exception& e) {
                    console->printLine("Failed to get call id from redial");
                }
                console->printRuler();
            }
        }
    });

    return true;
}

void PhoneCallControllerHandler::answer(const std::string& payload) {
    log(logger::LoggerHandler::Level::INFO, "answer:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }

    activity->runOnUIThread([=]() {
        // Assume we accepted successfully
        m_callState = CallState::ACTIVE;
        m_callError = "";
        updateCallId(payload);
        auto callId = getCallId(m_currentCall);
        callStateChanged(m_callState, callId);
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Phone call answered: ", getPhoneNumber(m_currentCall));
            console->printLine("Phone call state:    ", callStateToString());
            console->printLine("Phone call id:       ", callId);
            console->printRuler();
        }
    });
}

void PhoneCallControllerHandler::stop(const std::string& payload) {
    log(logger::LoggerHandler::Level::INFO, "stop:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    updateCallId(payload);
    activity->runOnUIThread([=]() {
        m_callState = CallState::IDLE;
        m_callError = "";
        auto callId = getCallId(m_currentCall);
        callStateChanged(m_callState, callId);
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Phone call stopped: ", getPhoneNumber(m_currentCall));
            console->printLine("Phone call state:   ", callStateToString());
            console->printLine("Phone call id:      ", callId);
            console->printRuler();
        }
    });
}

void PhoneCallControllerHandler::sendDTMF(const std::string& payload) {
    log(logger::LoggerHandler::Level::INFO, "sendDTMF:payload=" + payload);
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    json j = json::parse(payload);
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            std::string signal = j["signal"];
            console->printRuler();
            console->printLine("Send DTMF:         ", signal);
            console->printLine("Phone call number: ", getPhoneNumber(m_currentCall));
            console->printLine("Phone call state:  ", callStateToString());
            console->printLine("Phone call id:     ", getCallId(m_currentCall));
            console->printRuler();
        }
    });
}

void PhoneCallControllerHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
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
    activity->registerObserver(Event::onPhoneCallControllerConnectionStateChanged, [=](const std::string& value) {
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
        if (auto console = m_console.lock()) {
            // A feature of the calling device changed.
            console->printRuler();
            console->printLine("Phone connection state: " + value);
            if (ConnectionStateEnumerator.at(value) == ConnectionState::CONNECTED) {
                std::unordered_map<CallingDeviceConfigurationProperty, bool> config;
                config[CallingDeviceConfigurationProperty::DTMF_SUPPORTED] = true;
                deviceConfigurationUpdated(config);
                console->printLine("DTMF is supported");
            } else {
                auto callId = getCallId(m_currentCall);
                if (!callId.empty()) {
                    if (m_callState != CallState::IDLE) {
                        callFailed(callId, CallError::OTHER, "Phone disconnected");
                    }
                    callStateChanged(CallState::IDLE, callId);
                }
                m_callState = CallState::IDLE;
                updatePayload();
            }
            console->printRuler();
        }
        // Clear call error on phone connect/disconnect
        m_callError = "";
        return true;
    });

    // callStateChanged
    activity->registerObserver(Event::onPhoneCallControllerCallStateChanged, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerCallStateChanged:" + value);
        static std::regex r("([^/]+)(?:/([^/]+)(?:/([^/]*))?)?", std::regex::optimize);
        std::smatch sm{};
        if (!std::regex_match(value, sm, r)) {
            log(logger::LoggerHandler::Level::ERROR,
                "onPhoneCallControllerCallStateChanged incorrect format for value: " + value);
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

        // Call state to set to
        std::string callStateStr = sm[1];

        if (CallStateEnumerator.count(callStateStr) == 0) {
            log(logger::LoggerHandler::Level::ERROR,
                "onPhoneCallControllerCallStateChanged invalid call state passed: " + callStateStr);
            return false;
        }

        auto callState = CallStateEnumerator.at(callStateStr);

        // Create a new call
        if (m_currentCall.empty() || callState == CallState::INBOUND_RINGING) {
            log(logger::LoggerHandler::Level::INFO,
                "onPhoneCallControllerCallStateChanged created call payload since it was not available");
            createCall();
        }

        // Override callid if present in menu
        std::string callId = sm[2];

        // If call id still does not exist
        if (callId.empty()) {
            // Use call id from last call
            callId = getCallId(m_currentCall);
        } else {
            updateCallId(callId);
        }

        // Update caller id
        std::string callerId = sm[3];
        if (!callerId.empty()) {
            updateCallerId(callerId);
        }

        // Update outgoing call
        if (callState == CallState::OUTBOUND_RINGING) {
            m_outgoingCall = m_currentCall;
        }

        // Update call state globally
        m_callState = callState;
        m_callError = "";

        callStateChanged(m_callState, callId, callerId);

        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Phone call number: ", getPhoneNumber(m_currentCall));
            console->printLine("Phone call state : ", callStateToString());
            console->printRuler();
        }

        log(logger::LoggerHandler::Level::VERBOSE,
            "Changing call state. State: " + callStateStr + " CallId: " + callId + " CallerId: " + callerId);
        return true;
    });

    // callFailed
    activity->registerObserver(Event::onPhoneCallControllerCallFailed, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerCallFailed:" + value);

        // clang-format off
        static const std::map<std::string, CallError> CallErrorEnumerator{
            {"NO_CARRIER", CallError::NO_CARRIER},
            {"BUSY", CallError::BUSY},
            {"NO_ANSWER", CallError::NO_ANSWER},
            {"NO_NUMBER_FOR_REDIAL", CallError::NO_NUMBER_FOR_REDIAL},
            {"OTHER", CallError::OTHER}
        };
        // clang-format on

        static std::regex r("(.+)/(.+)/(.+)", std::regex::optimize);
        std::smatch sm{};
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 3)) {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Invalid value provided in menu. Format should be `callId/code/message`\n");
                console->printLine("Valid code values are:");
                for (auto it = CallErrorEnumerator.begin(); it != CallErrorEnumerator.end(); ++it) {
                    console->printLine("\t" + it->first);
                }
                console->printRuler();
            }

            return false;
        }

        if (auto console = m_console.lock()) {
            if (CallErrorEnumerator.count(sm[2]) == 0) {
                console->printRuler();
                console->printLine("Invalid call code: ", sm[2]);
                console->printLine("Valid code values are:");
                for (auto it = CallErrorEnumerator.begin(); it != CallErrorEnumerator.end(); ++it) {
                    console->printLine("\t" + it->first);
                }
                console->printRuler();
                return false;
            }

            // Save call error and reset call state
            m_callState = CallState::IDLE;
            m_callError = sm[2];

            auto callId = getCallId(m_currentCall);
            callStateChanged(m_callState, callId);

            callFailed(sm[1], CallErrorEnumerator.at(sm[2]), sm[3]);

            console->printRuler();
            console->printLine("Simulating failed call\n");
            console->printLine("Call id           : ", sm[1]);
            console->printLine("Call error        : ", sm[2]);
            console->printLine("Call error message: ", sm[3]);
            console->printRuler();
        }

        return true;
    });

    // callerIdReceived
    activity->registerObserver(Event::onPhoneCallControllerCallerIdReceived, [=](const std::string& value) {
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
    activity->registerObserver(Event::onPhoneCallControllerSendDTMFSucceeded, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerSendDTMFSucceeded:" + value);
        sendDTMFSucceeded(value);
        return true;
    });

    // sendDTMFFailed
    activity->registerObserver(Event::onPhoneCallControllerSendDTMFFailed, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerSendDTMFFailed:" + value);

        /// Regular expression for call state
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

    // Show payload
    activity->registerObserver(Event::onPhoneCallControllerShowPayload, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPhoneCallControllerShowPayload:");
        showPayload();
        return true;
    });
}

std::string PhoneCallControllerHandler::getPhoneNumber(const json& payload) {
    if (!payload.empty()) {
        return payload["callee"]["defaultContactAddress"]["value"];
    }

    return "";
}

std::string PhoneCallControllerHandler::getCallId(const json& payload) {
    if (!payload.empty()) {
        return payload["callId"];
    }

    return "";
}

void PhoneCallControllerHandler::updateCallId(const std::string& payload) {
    if (!m_currentCall.empty()) {
        try {
            json updated = json::parse(payload);
            m_currentCall["callId"] = updated["callId"];
        } catch (std::exception& e) {
            m_currentCall["callId"] = payload;
        }
    }
}

void PhoneCallControllerHandler::updateCallerId(const std::string& payload) {
    if (!m_currentCall.empty()) {
        try {
            json updated = json::parse(payload);
            m_currentCall["callerId"] = updated["callerId"];
        } catch (std::exception& e) {
            m_currentCall["callerId"] = payload;
        }
    }
}

void PhoneCallControllerHandler::showPayload() {
    if (auto console = m_console.lock()) {
        console->printRuler();

        if (!m_currentCall.empty()) {
            console->printLine("Current call payload:\n" + m_currentCall.dump(4));
        } else {
            console->printLine("Current call payload is empty\n");
        }
        console->printLine("Phone call state        : " + callStateToString());
        console->printLine("Last phone number dialed: " + getPhoneNumber(m_outgoingCall));
        if (!m_callError.empty()) {
            console->printLine("Last phone call error   : " + m_callError);
        }

        console->printRuler();
    }
}

std::string PhoneCallControllerHandler::configurationFeatureToString(CallingDeviceConfigurationProperty feature) {
    switch (feature) {
        case CallingDeviceConfigurationProperty::DTMF_SUPPORTED:
            return "DTMF_SUPPORTED";
        default:
            return "";
    }
}

std::string PhoneCallControllerHandler::callStateToString() {
    static const std::map<CallState, std::string> callStateMap{{CallState::IDLE, "IDLE"},
                                                               {CallState::DIALING, "DIALING"},
                                                               {CallState::OUTBOUND_RINGING, "OUTBOUND_RINGING"},
                                                               {CallState::ACTIVE, "ACTIVE"},
                                                               {CallState::CALL_RECEIVED, "CALL_RECEIVED"},
                                                               {CallState::INBOUND_RINGING, "INBOUND_RINGING"}};

    return callStateMap.at(m_callState);
}

void PhoneCallControllerHandler::createCall() {
    m_currentCall = {};
    m_currentCall["callId"] = createCallId();
    m_currentCall["callerId"] = "John Doe";
    m_currentCall["callee"]["defaultContactAddress"]["value"] = "555-555-555";
    m_currentCall["callee"]["defaultContactAddress"]["protocol"] = "PSTN";
    m_currentCall["callee"]["details"] = "Mobile";
}

void PhoneCallControllerHandler::updatePayload(const std::string& payload) {
    try {
        if (!payload.empty()) {
            auto j = json::parse(payload);
            m_currentCall = j;
        } else {
            m_currentCall = {};
        }
    } catch (std::exception& e) {
        log(logger::LoggerHandler::Level::ERROR, "PhoneCallController failed to parse JSON payload: " + payload);
    }
}

std::string PhoneCallControllerHandler::waitForAsyncReply(const std::string& messageId) {
    // create the promise for the phone call controller reply message to fulfill
    std::shared_ptr<PhoneCallControllerPromise> promise = std::make_shared<PhoneCallControllerPromise>();

    // create a future to receive the promised phone call controller reply message when it is received
    std::shared_future<std::string> future(promise->get_future());

    std::string success = "";
    try {
        addReplyMessagePromise(messageId, promise);
        ThrowIfNot(
            future.wait_for(std::chrono::milliseconds(ASYNC_REPLY_TIMEOUT)) == std::future_status::ready,
            "replyMessageTimeout:id=" + messageId);
        ThrowIfNot(future.valid(), "invalidMessageResponse");
        success = future.get();
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "waitForAsyncReply " + std::string(ex.what()));
        promise->set_exception(std::current_exception());
    }
    removeReplyMessagePromise(messageId);
    return success;
}

void PhoneCallControllerHandler::addReplyMessagePromise(
    const std::string& messageId,
    std::shared_ptr<PhoneCallControllerPromise> promise) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        ThrowIf(m_promiseMap.find(messageId) != m_promiseMap.end(), "messageIdAlreadyExists");

        // add the promise to the promise map
        m_promiseMap[messageId] = promise;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "addReplyMessagePromise " + std::string(ex.what()));
    }
}

void PhoneCallControllerHandler::removeReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        // remove the promise from the promise map
        m_promiseMap.erase(messageId);
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "removeReplyMessagePromise " + std::string(ex.what()));
    }
}

std::shared_ptr<PhoneCallControllerHandler::PhoneCallControllerPromise> PhoneCallControllerHandler::
    getReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        return it->second;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "getReplyMessagePromise " + std::string(ex.what()));
        return nullptr;
    }
}

}  // namespace phoneControl
}  // namespace sampleApp
