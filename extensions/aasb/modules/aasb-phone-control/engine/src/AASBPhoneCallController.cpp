/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/PhoneCallController/AASBPhoneCallController.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/JSON/JSON.h>

#include <AASB/Message/PhoneCallController/PhoneCallController/AnswerMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallerIdReceivedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallError.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallFailedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallingDeviceConfigurationProperty.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallState.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallStateChangedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/ConnectionState.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/ConnectionStateChangedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CreateCallIdMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CreateCallIdMessageReply.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/DeviceConfigurationUpdatedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/DialMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/DTMFError.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/RedialMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/SendDTMFFailedMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/SendDTMFMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/SendDTMFSucceededMessage.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/StopMessage.h>

#include <future>
#include <rapidjson/pointer.h>

namespace aasb {
namespace engine {
namespace phoneCallController {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.phoneCallController.AASBPhoneCallController");

// aliases
using Message = aace::engine::aasb::Message;

// Topic @c TOPIC_PCC
static const std::string TOPIC_PCC = "PhoneCallController";

// Actions for @c TOPIC_PCC topic
static const std::string ACTION_PCC_DIAL = "dial";
static const std::string ACTION_PCC_REDIAL = "redial";
static const std::string ACTION_PCC_ANSWER = "answer";
static const std::string ACTION_PCC_STOP = "stop";
static const std::string ACTION_PCC_SEND_DTMF = "send_dtmf";
static const std::string ACTION_PCC_CONNECTION_STATE_CHANGED = "connection_state_changed";
static const std::string ACTION_PCC_CALL_STATE_CHANGED = "call_state_changed";
static const std::string ACTION_PCC_CALL_FAILED = "call_failed";
static const std::string ACTION_PCC_CALLER_ID_RECEIVED = "caller_id_received";
static const std::string ACTION_PCC_SEND_DTMF_SUCCEEDED = "send_dtmf_succeeded";
static const std::string ACTION_PCC_SEND_DTMF_FAILED = "send_dtmf_failed";
static const std::string ACTION_PCC_DEVICE_CONFIGURATION_UPDATED = "configuration_updated";
static const std::string ACTION_PCC_CREATE_CALL_ID = "create_call_id";

using ConnectionState = aace::phoneCallController::PhoneCallController::ConnectionState;
using CallState = aace::phoneCallController::PhoneCallController::CallState;
using CallError = aace::phoneCallController::PhoneCallController::CallError;
using DTMFError = aace::phoneCallController::PhoneCallController::DTMFError;
using CallingDeviceConfigurationProperty =
    aace::phoneCallController::PhoneCallController::CallingDeviceConfigurationProperty;

// Enum to string
static const std::unordered_map<std::string, ConnectionState> ConnectionStateEnumerator{
    {"CONNECTED", ConnectionState::CONNECTED},
    {"DISCONNECTED", ConnectionState::DISCONNECTED}};

static const std::unordered_map<std::string, CallState> CallStateEnumerator{
    {"IDLE", CallState::IDLE},
    {"DIALING", CallState::DIALING},
    {"OUTBOUND_RINGING", CallState::OUTBOUND_RINGING},
    {"ACTIVE", CallState::ACTIVE},
    {"CALL_RECEIVED", CallState::CALL_RECEIVED},
    {"INBOUND_RINGING", CallState::INBOUND_RINGING}};

static const std::unordered_map<std::string, CallError> CallErrorEnumerator{
    {"NO_CARRIER", CallError::NO_CARRIER},
    {"BUSY", CallError::BUSY},
    {"NO_ANSWER", CallError::NO_ANSWER},
    {"NO_NUMBER_FOR_REDIAL", CallError::NO_NUMBER_FOR_REDIAL},
    {"OTHER", CallError::OTHER}};

static const std::unordered_map<std::string, DTMFError> DTMFErrorEnumerator{
    {"CALL_NOT_IN_PROGRESS", DTMFError::CALL_NOT_IN_PROGRESS},
    {"DTMF_FAILED", DTMFError::DTMF_FAILED}};

static const std::unordered_map<std::string, CallingDeviceConfigurationProperty> ConfigPropertyEnumerator{
    {"DTMF_SUPPORTED", CallingDeviceConfigurationProperty::DTMF_SUPPORTED}};

bool AASBPhoneCallController::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBPhoneCallController> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::phoneCallController::phoneCallController::ConnectionStateChangedMessage::topic(),
            aasb::message::phoneCallController::phoneCallController::ConnectionStateChangedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::phoneCallController::phoneCallController::ConnectionStateChangedMessage::Payload
                        payload = nlohmann::json::parse(message.payload());
                    sp->connectionStateChanged(static_cast<ConnectionState>(payload.state));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ConnectionStateChangedMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::phoneCallController::phoneCallController::CallStateChangedMessage::topic(),
            aasb::message::phoneCallController::phoneCallController::CallStateChangedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::phoneCallController::phoneCallController::CallStateChangedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->callStateChanged(static_cast<CallState>(payload.state), payload.callId, payload.callerId);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "CallStateChangedMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::phoneCallController::phoneCallController::CallFailedMessage::topic(),
            aasb::message::phoneCallController::phoneCallController::CallFailedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::phoneCallController::phoneCallController::CallFailedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->callFailed(payload.callId, static_cast<CallError>(payload.code), payload.message);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "CallFailedMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::phoneCallController::phoneCallController::CallerIdReceivedMessage::topic(),
            aasb::message::phoneCallController::phoneCallController::CallerIdReceivedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::phoneCallController::phoneCallController::CallerIdReceivedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->callerIdReceived(payload.callId, payload.callerId);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "CallerIdReceivedMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::phoneCallController::phoneCallController::SendDTMFSucceededMessage::topic(),
            aasb::message::phoneCallController::phoneCallController::SendDTMFSucceededMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::phoneCallController::phoneCallController::SendDTMFSucceededMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->sendDTMFSucceeded(payload.callId);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SendDTMFSucceededMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::phoneCallController::phoneCallController::SendDTMFFailedMessage::topic(),
            aasb::message::phoneCallController::phoneCallController::SendDTMFFailedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::phoneCallController::phoneCallController::SendDTMFFailedMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->sendDTMFFailed(payload.callId, static_cast<DTMFError>(payload.code), payload.message);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SendDTMFFailedMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::phoneCallController::phoneCallController::DeviceConfigurationUpdatedMessage::topic(),
            aasb::message::phoneCallController::phoneCallController::DeviceConfigurationUpdatedMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::phoneCallController::phoneCallController::DeviceConfigurationUpdatedMessage::Payload
                        payload = nlohmann::json::parse(message.payload());
                    auto configurationMapString = nlohmann::json::parse(payload.configurationMap);

                    std::unordered_map<CallingDeviceConfigurationProperty, bool> configurationMap;

                    for (auto it = configurationMapString.begin(); it != configurationMapString.end(); ++it) {
                        configurationMap.insert(
                            {static_cast<CallingDeviceConfigurationProperty>(
                                 aasb::message::phoneCallController::toCallingDeviceConfigurationProperty(it.key())),
                             it.value()});
                    }

                    sp->deviceConfigurationUpdated(configurationMap);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "DeviceConfigurationUpdatedMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::phoneCallController::phoneCallController::CreateCallIdMessage::topic(),
            aasb::message::phoneCallController::phoneCallController::CreateCallIdMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::phoneCallController::phoneCallController::CreateCallIdMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    auto m_messageBroker_lock = sp->m_messageBroker.lock();
                    ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

                    aasb::message::phoneCallController::phoneCallController::CreateCallIdMessageReply
                        createCallIdMessageReply;
                    createCallIdMessageReply.header.messageDescription.replyToId = message.messageId();
                    createCallIdMessageReply.payload.callId = sp->createCallId();
                    m_messageBroker_lock->publish(createCallIdMessageReply.toString()).send();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "CreateCallIdMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AASBPhoneCallController> AASBPhoneCallController::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBPhoneCallController>(new AASBPhoneCallController());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBPhoneCallControllerFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBPhoneCallController::dial(const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::phoneCallController::phoneCallController::DialMessage message;
        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBPhoneCallController::redial(const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::phoneCallController::phoneCallController::RedialMessage message;
        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AASBPhoneCallController::answer(const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::phoneCallController::phoneCallController::AnswerMessage message;
        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBPhoneCallController::stop(const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::phoneCallController::phoneCallController::StopMessage message;
        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBPhoneCallController::sendDTMF(const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::phoneCallController::phoneCallController::SendDTMFMessage message;
        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace phoneCallController
}  // namespace engine
}  // namespace aasb
