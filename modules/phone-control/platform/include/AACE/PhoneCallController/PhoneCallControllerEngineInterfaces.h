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

#ifndef AACE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_ENGINE_INTERFACE_H
#define AACE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_ENGINE_INTERFACE_H

namespace aace {
namespace phoneCallController {

/**
 * PhoneCallControllerEngineInterface
 */
class PhoneCallControllerEngineInterface {
public:
    /**
     * Describes the state of connection to a calling device
     * 
     * @sa PhoneCallController::connectionStateChanged
     */
    enum class ConnectionState {

        /**
         * A calling device is connected.
         */
        CONNECTED,

        /**
         * No calling device is connected.
         */
        DISCONNECTED
    };

    /**
     * Describes the state of a call
     * 
     * @sa PhoneCallController::callStateChanged
     */
    enum class CallState {

        /**
         * The call is not in an active state.
         * The following are possible transitions to IDLE state:
         * @li While in DIALING or OUTBOUND_RINGING, the user requested to hang up or cancel the outbound call
         * @li While in ACTIVE state, the user hangs up the active call.
         * @li While in CALL_RECEIVED or INBOUND_RINGING state, the user requested to ignore or reject the inbound call.
         */
        IDLE,

        /**
         * The outbound call is initiated by the user. Call setup is in progress.
         */
        DIALING,

        /**
         * The outbound call has been set up, and the remote party is alerted.
         */
        OUTBOUND_RINGING,

        /**
         * The call is active, and media is being transmitted between the caller and remote party.
         * The following are possible transitions to ACTIVE state:
         * @li While in DIALING or OUTBOUND_RINGING, outbound call setup is sucessfully completed.
         * @li While in INBOUND_RINGING, the user accepted the call, and setup is completed.
         * @li When transitioning from DISCONNECTED to CONNECTED, and the calling device is already in an active call.
         */
        ACTIVE,

        /**
         * An alert for the inbound call has been received.
         */
        CALL_RECEIVED,

        /**
         * The inbound call is ringing.
         */
        INBOUND_RINGING
    };

    /**
     * Describes a configuration property of a connected calling device.
     * Configure properties with @c PhoneCallController::deviceConfigurationUpdated
     */
    enum class CallingDeviceConfigurationProperty {

        /**
         * Whether the device supports DTMF signaling.
         * Set @c true to indicate support for DTMF.
         * Set @c false when DTMF signaling is not implemented or the current calling device configuration does not
         * support DTMF.
         * Default setting: @c false
         */
        DTMF_SUPPORTED
    };

    /**
     * Describes an error for a failed call
     * 
     * @sa PhoneCallController::callFailed
     */
    enum class CallError {

        /**
         * No carrier is available on the calling device.
         */
        NO_CARRIER,

        /**
         * The calling device is busy when setting up an outbound call, such as when a call is
         * already in progress.
         */
        BUSY,

        /**
         * The remote party did not answer the call.
         */
        NO_ANSWER,

        /**
         * Redial was requested, but there is no previously dialed number available.
         */
        NO_NUMBER_FOR_REDIAL,

        /** 
         * Generic error
         */
        OTHER
    };

    /**
     * Describes an error preventing a DTMF signal from being delivered
     * 
     * @sa PhoneCallController::sendDTMFFailed
     */
    enum class DTMFError {

        /**
         * There is no active call through which a DTMF signal can be sent.
         */
        CALL_NOT_IN_PROGRESS,

        /**
         * Generic DTMF error
         */
        DTMF_FAILED
    };

    virtual void onConnectionStateChanged(ConnectionState state) = 0;
    virtual void onCallStateChanged(CallState state, const std::string& callId, const std::string& callerId) = 0;
    virtual void onCallFailed(const std::string& callId, CallError code, const std::string& message) = 0;
    virtual void onCallerIdReceived(const std::string& callId, const std::string& callerId) = 0;
    virtual void onSendDTMFSucceeded(const std::string& callId) = 0;
    virtual void onSendDTMFFailed(const std::string& callId, DTMFError code, const std::string& message) = 0;
    virtual void onDeviceConfigurationUpdated(
        std::unordered_map<PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty, bool>
            configurationMap) = 0;
    virtual std::string onCreateCallId() = 0;
};

}  // namespace phoneCallController
}  // namespace aace

namespace std {
template <>
struct hash<aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty> {
    size_t operator()(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty x) const {
        return hash<int>()(static_cast<int>(x));
    }
};
}  // namespace std

#endif
