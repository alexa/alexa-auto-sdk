/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_H
#define AACE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_H

#include <string>
#include <unordered_map>

#include "AACE/Core/PlatformInterface.h"
#include "PhoneCallControllerEngineInterfaces.h"

/** @file */

namespace aace {
namespace phoneCallController {

/**
 * PhoneCallController should be extended to allow a user to use Alexa to interact with a calling device 
 * such as a connected mobile phone. It provides interfaces for controlling inbound and outbound calls
 * and for notifying the Engine of the state of a call session and the state of platform connection 
 * to a calling device.
 *
 * The platform implementation is responsible for managing the lifecycle and user experience of a call session
 * and connection to the calling device.
 */
class PhoneCallController : public aace::core::PlatformInterface {
protected:
    PhoneCallController() = default;

public:
    /**
     * Describes the state of connection to a calling device
     */
    using ConnectionState = aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState;

    /**
     * Describes the state of a call
     */
    using CallState = aace::phoneCallController::PhoneCallControllerEngineInterface::CallState;

    /**
     * Describes a configuration property of a connected calling device
     */
    using CallingDeviceConfigurationProperty =
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty;

    /**
     * Describes an error for a failed call
     */
    using CallError = aace::phoneCallController::PhoneCallControllerEngineInterface::CallError;

    /** 
     * Describes an error preventing a DTMF signal from being delivered
     */
    using DTMFError = aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError;

    virtual ~PhoneCallController();

    /**
     * Notifies the platform implementation to initiate an outgoing phone call
     * to the destination address
     * 
     * @param [in] payload Details of the dial request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code{.json}
     *  {
     *     "callId": "{{STRING}}",
     *     "callee": {
     *       "details": "{{STRING}}",
     *       "defaultContactAddress": {
     *         "protocol": "{{STRING}}",
     *         "format": "{{STRING}}",
     *         "value": "{{STRING}}"
     *       },
     *       "alternativeContactAddresses": [{
     *         "protocol": "{{STRING}}",
     *         "format": "{{STRING}}",
     *         "value": {{STRING}}
     *       }]
     *     },
     *   }
     * }
     * @endcode 
     * @li callId (required): A unique identifier for the call
     * @li callee (required): The destination of the outgoing call
     * @li callee.details (optional): Descriptive information about the callee
     * @li callee.defaultContactAddress (required): The default address to use for calling the callee
     * @li callee.alternativeContactAddresses (optional): An array of alternate addresses for the callee
     * @li address.protocol (required): The protocol for this address of the callee. One of PSTN, SIP, H.323
     * @li address.format (optional): The format for this address of the callee. One of E.164, E.163, E.123, MICROSOFT, DIN5008, RAW
     * @li address.value (required): The address of the callee.
     *
     * @return @c true if the platform implementation successfully handled the call
     */
    virtual bool dial(const std::string& payload) = 0;

    /**
     * Notifies the platform implementation to redial the last called phone number.
     * 
     * After returning @c true, if no stored number is available to be redialed, @c PhoneCallController::callFailed with 
     * @c CallError::NO_NUMBER_FOR_REDIAL should be called.
     * 
     * @param [in] payload Details of the redial request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code{.json}
     * {
     *   "callId": "{{STRING}}"
     * }
     * @endcode 
     * @li callId (required): A unique identifier for the call
     *
     * @return @c true if the platform implementation successfully handled the call
     */
    virtual bool redial(const std::string& payload) = 0;

    /**
     * Notifies the platform implementation to answer an inbound call
     * 
     * @param [in] payload Details of the answer request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code{.json}
     * {
     *   "callId": "{{STRING}}",
     * }
     * @endcode 
     * @li callId (required): The unique identifier for the call to answer
     */
    virtual void answer(const std::string& payload) = 0;

    /**
     * Notifies the platform implementation to end an ongoing call or stop inbound or outbound call setup
     * 
     * @param [in] payload Details of the stop request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code{.json}
     * {
     *   "callId": "{{STRING}}"
     * }
     * @endcode 
     * @li callId (required): The unique identifier for the call to be stopped
     */
    virtual void stop(const std::string& payload) = 0;

    /**
     * Notifies the platform implementation to send a DTMF signal to the calling device
     * 
     * @param [in] payload Details of the DTMF request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code{.json}
     * {
     *   "callId": "{{STRING}}",
     *   "signal": "{{STRING}}"
     * }
     * @endcode 
     * @li callId (required): The unique identifier for the call
     * @li signal (required): The DTMF string to be sent to the calling device associated with the callId
     */
    virtual void sendDTMF(const std::string& payload) = 0;

    /**
     * Notifies the Engine of a change in connection to a calling device
     * 
     * @param [in] state The state of connection to a calling device
     */
    void connectionStateChanged(ConnectionState state);

    /**
     * Notifies the Engine of a change in the state of an ongoing call
     * 
     * @param [in] state The state of the call
     * @param [in] callId The unique identifier associated with the call
     * @param [in] callerId The identifier for a contact. May be included for @c CallState::CALL_RECEIVED
     */
    void callStateChanged(CallState state, const std::string& callId, const std::string& callerId = "");

    /**
     * Notifies the Engine of an error related to a call
     * 
     * @param [in] callId The unique identifier for the call associated with the error
     * @param [in] code The error type
     * @param [in] message A description of the error
     */
    void callFailed(const std::string& callId, CallError code, const std::string& message = "");

    /**
     * Notifies the Engine that a caller id was received for an inbound call
     * 
     * @param [in] callId The unique identifier for the call associated with the callId
     * @param [in] callerId The caller's identifier or phone number
     */
    void callerIdReceived(const std::string& callId, const std::string& callerId);

    /** 
     * Notifies the Engine that sending the DTMF signal succeeded.
     * 
     * @param [in] callId The unique identifier for the associated call
     * 
     * @sa PhoneCallController::sendDTMF
     */
    void sendDTMFSucceeded(const std::string& callId);

    /** 
     * Notifies the Engine that the DTMF signal could not be delivered to the remote party
     * 
     * @param [in] callId The unique identifier for the associated call
     * @param [in] code The error type
     * @param [in] message A description of the error
     * 
     * @sa PhoneCallController::sendDTMF
     */
    void sendDTMFFailed(const std::string& callId, DTMFError code, const std::string& message = "");

    /**
     * Notifies the Engine of the calling feature configuration of the connected calling device.
     * The configuration data may change if, for example, the connection mechanism to the calling device changes.
     * The provided configuration will override the default or previous configuration.
     * 
     * See @c PhoneCallController::CallingDeviceConfigurationProperty for a 
     * description of each configurable feature
     * 
     * @param [in] configurationMap A map of configuration properties to the boolean state of the properties
     */
    void deviceConfigurationUpdated(std::unordered_map<CallingDeviceConfigurationProperty, bool> configurationMap);

    /**
     * Generates a unique identifier for a call
     */
    std::string createCallId();

    /**
     * @internal
     * Sets the Engine interface delegate
     * 
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface(std::shared_ptr<PhoneCallControllerEngineInterface> phoneCallControllerEngineInterface);

private:
    std::shared_ptr<PhoneCallControllerEngineInterface> m_phoneCallControllerEngineInterface;
};

}  // namespace phoneCallController
}  // namespace aace

#endif
