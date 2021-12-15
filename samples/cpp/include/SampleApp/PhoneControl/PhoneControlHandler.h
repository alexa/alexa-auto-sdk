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

#ifndef SAMPLEAPP_PHONECONTROL_PHONECONTROLHANDLER_H
#define SAMPLEAPP_PHONECONTROL_PHONECONTROLHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AASB/Message/PhoneCallController/PhoneCallController/CallError.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallState.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/CallingDeviceConfigurationProperty.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/ConnectionState.h>
#include <AASB/Message/PhoneCallController/PhoneCallController/DTMFError.h>

#include <AACE/Core/MessageBroker.h>

#include <unordered_map>
#include <future>
#include <utility>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace phoneControl {

using namespace aasb::message::phoneCallController::phoneCallController;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PhoneCallControlHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class PhoneCallControllerHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    PhoneCallControllerHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<PhoneCallControllerHandler> {
        return std::shared_ptr<PhoneCallControllerHandler>(new PhoneCallControllerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the AnswerMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAnswerMessage(const std::string& message);

    /**
     * Handles the DialMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleDialMessage(const std::string& message);

    /**
     * Handles the RedialMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleRedialMessage(const std::string& message);

    /**
     * Handles the SendDTMFMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSendDMTFMessage(const std::string& message);

    /**
     * Handles the StopMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleStopMessage(const std::string& message);

    /**
     * Handles the CreateCallIdReplyMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleCreateCallIdReplyMessage(const std::string& message);

    /**
     * Publishes the CallFailed message to notify the Engine of an error related to a call.
     * 
     * @param [in] callId The unique identifier for the call associated with the error
     * @param [in] code The error type
     * @param [in] message A description of the error
     */
    void callFailed(const std::string& callId, CallError code, const std::string& message = "");

    /**
     * Publishes the CallStateChanged message to notify the Engine of a change in the state of an ongoing call.
     * 
     * @param [in] state The state of the call
     * @param [in] callId The unique identifier associated with the call
     * @param [in] callerId The identifier for a contact
     */
    void callStateChanged(CallState state, const std::string& callId, const std::string& callerId = "");

    /**
     * Publishes the CallerIdReceived message to notify the Engine that a caller id was received for an inbound call.
     * 
     * @param [in] callId The unique identifier for the call associated with the callId
     * @param [in] callerId The caller's identifier or phone number
     */
    void callerIdReceived(const std::string& callId, const std::string& callerId);

    /**
     * Publishes the CreateCallId message to generate a unique identifier for a call.
     *
     * A reply message is expected from the Engine. The default wait time for the reply is 1000 ms.
     * If the reply is not received within that time, a timeout occurs.
     */
    std::string createCallId();

    /**
     * Publishes the DeviceConfigurationUpdated message to notify the Engine of the calling feature configuration of the connected calling device.
     * 
     * @param [in] configurationMap A map of configuration properties to the boolean state of the properties
     */
    void deviceConfigurationUpdated(std::unordered_map<CallingDeviceConfigurationProperty, bool> configurationMap);

    /** 
     * Publishes the SendSTMFSucceeded message to notify the Engine that sending the DTMF signal succeeded.
     * 
     * @param [in] callId The unique identifier for the associated call
     */
    void sendDTMFSucceeded(const std::string& callId);

    /** 
     * Publishes the SendSTMFFailed message to notify the Engine that the DTMF signal could not be delivered to the remote party.
     * 
     * @param [in] callId The unique identifier for the associated call
     * @param [in] code The error type
     * @param [in] message A description of the error
     */
    void sendDTMFFailed(const std::string& callId, DTMFError code, const std::string& message = "");

    /**
     * Provides the implementation to initiate an outgoing phone call to the destination address.
     * 
     * @param [in] payload Details of the dial request in structured JSON format
     */
    auto dial(const std::string& payload) -> bool;

    /**
     * Provides the implementation to redial the last called phone number.
     * 
     * @param [in] payload Details of the redial request in structured JSON format
     */
    auto redial(const std::string& payload) -> bool;

    /**
     * Provides the implementation to answer an inbound call.
     * 
     * @param [in] payload Details of the answer request in structured JSON format
     */
    auto answer(const std::string& payload) -> void;

    /**
     * Provides the implementation to end an ongoing call or stop inbound or outbound call setup.
     * 
     * @param [in] payload Details of the stop request in structured JSON format
     */
    auto stop(const std::string& payload) -> void;

    /**
     * Provides the implementation to send a DTMF signal to the calling device.
     * 
     * @param [in] payload Details of the DTMF request in structured JSON format
     */
    auto sendDTMF(const std::string& payload) -> void;

public:
    /**
     * Publishes the ConnectionStateChanged message to notify the Engine of a change in connection to a calling device.
     * 
     * @param [in] state The state of connection to a calling device
     */
    void connectionStateChanged(ConnectionState state);

    /**
     * Helper method to display the current payload data.
     */
    void showPayload();

private:
    void createCall();
    void updateCallId(const std::string& payload);
    void updateCallerId(const std::string& value);
    void updatePayload(const std::string& payload = "");
    std::string getPhoneNumber(const json& payload);
    std::string getCallId(const json& payload);
    std::string callStateToString();
    std::string configurationFeatureToString(CallingDeviceConfigurationProperty feature);

private:
    using PhoneCallControllerPromise = std::promise<std::string>;
    std::mutex m_promise_map_access_mutex;
    std::unordered_map<std::string, std::shared_ptr<PhoneCallControllerPromise>> m_promiseMap;

    std::string waitForAsyncReply(const std::string& messageId);
    void addReplyMessagePromise(const std::string& messageId, std::shared_ptr<PhoneCallControllerPromise> promise);
    void removeReplyMessagePromise(const std::string& messageId);
    std::shared_ptr<PhoneCallControllerPromise> getReplyMessagePromise(const std::string& messageId);

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;

    /// Payload for current call incoming or outgoing
    json m_currentCall;

    /// Last number dialed
    json m_outgoingCall;

    /// Call state
    CallState m_callState;

    /// Call error
    std::string m_callError;
};

}  // namespace phoneControl
}  // namespace sampleApp

/**
 * Specialized hash function for keys of unordered map of configuration properties.
 */
namespace std {
template <>
struct hash<aasb::message::phoneCallController::phoneCallController::CallingDeviceConfigurationProperty> {
    size_t operator()(
        aasb::message::phoneCallController::phoneCallController::CallingDeviceConfigurationProperty x) const {
        return hash<int>()(static_cast<int>(x));
    }
};
}  // namespace std

#endif  // SAMPLEAPP_PHONECONTROL_PHONECONTROLHANDLER_H