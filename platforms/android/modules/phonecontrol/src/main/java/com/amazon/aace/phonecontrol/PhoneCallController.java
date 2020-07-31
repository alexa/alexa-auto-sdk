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

package com.amazon.aace.phonecontrol;

import com.amazon.aace.core.PlatformInterface;

import java.util.HashMap;

/**
 * PhoneCallController should be extended to allow a user to use Alexa to interact with a calling device
 * such as a connected mobile phone. It provides interfaces for controlling inbound and outbound calls
 * and for notifying the Engine of the state of a call session and the state of platform connection
 * to a calling device.
 *
 * The platform implementation is responsible for managing the lifecycle and user experience of a call session
 * and connection to the calling device.
 */
abstract public class PhoneCallController extends PlatformInterface {
    /**
     * Describes the state of connection to a calling device
     *
     * @sa PhoneCallController::connectionStateChanged
     */
    public enum ConnectionState {
        /**
         * A calling device is connected.
         * @hideinitializer
         */
        CONNECTED("CONNECTED"),
        /**
         * No calling device is connected.
         * @hideinitializer
         */
        DISCONNECTED("DISCONNECTED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ConnectionState(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Describes the state of a call
     *
     * @sa PhoneCallController::callStateChanged
     */
    public enum CallState {
        /**
         * The call is not in an active state.
         * The following are possible transitions to IDLE state:
         * @li While in DIALING or OUTBOUND_RINGING, the user requested to hang up or cancel the outbound call
         * @li While in ACTIVE state, the user hangs up the active call.
         * @li While in CALL_RECEIVED or INBOUND_RINGING state, the user requested to ignore or reject the inbound call.
         *
         * @hideinitializer
         */
        IDLE("IDLE"),

        /**
         * The outbound call is initiated by the user. Call setup is in progress.
         *
         * @hideinitializer
         */
        DIALING("DIALING"),

        /**
         * The outbound call has been set up, and the remote party is alerted.
         *
         * @hideinitializer
         */
        OUTBOUND_RINGING("OUTBOUND_RINGING"),

        /**
         * The call is active, and media is being transmitted between the caller and remote party.
         * The following are possible transitions to ACTIVE state:
         * @li While in DIALING or OUTBOUND_RINGING, outbound call setup is sucessfully completed.
         * @li While in INBOUND_RINGING, the user accepted the call, and setup is completed.
         * @li When transitioning from DISCONNECTED to CONNECTED, and the calling device is already in an active call.
         *
         * @hideinitializer
         */
        ACTIVE("ACTIVE"),

        /**
         * An alert for the inbound call has been received.
         *
         * @hideinitializer
         */
        CALL_RECEIVED("CALL_RECEIVED"),

        /**
         * The inbound call is ringing.
         *
         * @hideinitializer
         */
        INBOUND_RINGING("INBOUND_RINGING");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private CallState(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Describes a configuration property of a connected calling device.
     * Configure properties with @c PhoneCallController::deviceConfigurationUpdated
     */
    public enum CallingDeviceConfigurationProperty {
        /**
         * Whether the device supports DTMF signaling.
         * Set @c true to indicate support for DTMF.
         * Set @c false when DTMF signaling is not implemented or the current calling device configuration does not
         * support DTMF.
         * Default setting: @c false
         *
         * @hideinitializer
         */
        DTMF_SUPPORTED("DTMF_SUPPORTED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private CallingDeviceConfigurationProperty(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Describes an error for a failed call
     *
     * @sa PhoneCallController::callFailed
     */
    public enum CallError {
        /**
         * No carrier is available on the calling device.
         *
         * @hideinitializer
         */
        NO_CARRIER("NO_CARRIER"),

        /**
         * The calling device is busy when setting up an outbound call, such as when a call is
         * already in progress.
         *
         * @hideinitializer
         */
        BUSY("BUSY"),

        /**
         * The remote party did not answer the call.
         *
         * @hideinitializer
         */
        NO_ANSWER("NO_ANSWER"),

        /**
         * Redial was requested, but there is no previously dialed number available.
         *
         * @hideinitializer
         */
        NO_NUMBER_FOR_REDIAL("NO_NUMBER_FOR_REDIAL"),

        /**
         * Generic error
         *
         * @hideinitializer
         */
        OTHER("OTHER");
        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private CallError(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Describes an error preventing a DTMF signal from being delivered
     *
     * @sa PhoneCallController::sendDTMFFailed
     */
    public enum DTMFError {
        /**
         * There is no active call through which a DTMF signal can be sent.
         *
         * @hideinitializer
         */
        CALL_NOT_IN_PROGRESS("CALL_NOT_IN_PROGRESS"),

        /**
         * Generic DTMF error
         *
         * @hideinitializer
         */
        DTMF_FAILED("DTMF_FAILED");
        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private DTMFError(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public PhoneCallController() {}

    /**
     * Notifies the platform implementation to initiate an outgoing phone call
     * to the destination address
     *
     * @param  payload Details of the dial request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code {.json}
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
     * @li address.format (optional): The format for this address of the callee. One of E.164, E.163, E.123, MICROSOFT,
     * DIN5008, RAW
     * @li address.value (required): The address of the callee.
     *
     * @return @c true if the platform implementation successfully handled the call
     */
    public boolean dial(String payload) {
        return false;
    }

    /**
     * Notifies the platform implementation to redial the last called phone number.
     *
     * After returning @c true, if no stored number is available to be redialed, @c PhoneCallController::callFailed with
     * @c CallError::NO_NUMBER_FOR_REDIAL should be called.
     *
     * @param  payload Details of the redial request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code {.json}
     * {
     *   "callId": "{{STRING}}"
     * }
     * @endcode
     * @li callId (required): A unique identifier for the call
     *
     * @return @c true if the platform implementation successfully handled the call
     */
    public boolean redial(String payload) {
        return false;
    }

    /**
     * Notifies the platform implementation to answer an inbound call
     *
     * @param  payload Details of the answer request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code {.json}
     * {
     *   "callId": "{{STRING}}",
     * }
     * @endcode
     * @li callId (required): The unique identifier for the call to answer
     */
    public void answer(String payload) {}

    /**
     * Notifies the platform implementation to end an ongoing call or stop inbound or outbound call setup
     *
     * @param  payload Details of the stop request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code {.json}
     * {
     *   "callId": "{{STRING}}"
     * }
     * @endcode
     * @li callId (required): The unique identifier for the call to be stopped
     */
    public void stop(String payload) {}

    /**
     * Notifies the platform implementation to send a DTMF signal to the calling device
     *
     * @param  payload Details of the DTMF request in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code {.json}
     * {
     *   "callId": "{{STRING}}",
     *   "signal": "{{STRING}}"
     * }
     * @endcode
     * @li callId (required): The unique identifier for the call
     * @li signal (required): The DTMF string to be sent to the calling device associated with the callId
     */
    public void sendDTMF(String payload) {}

    /**
     * Notifies the Engine of a change in connection to a calling device
     *
     * @param  state The state of connection to a calling device
     */
    final protected void connectionStateChanged(ConnectionState state) {
        connectionStateChanged(getNativeRef(), state);
    }

    /**
     * Notifies the Engine of a change in the state of an ongoing call
     *
     * @param  state The state of the call
     * @param  callId The unique identifier associated with the call
     */
    final protected void callStateChanged(CallState state, String callId) {
        callStateChanged(getNativeRef(), state, callId, "");
    }

    /**
     * Notifies the Engine of a change in the state of an ongoing call
     *
     * @param  state The state of the call
     * @param  callId The unique identifier associated with the call
     * @param  callerId The identifier for a contact. May be included for @c CallState::CALL_RECEIVED
     */
    final protected void callStateChanged(CallState state, String callId, String callerId) {
        callStateChanged(getNativeRef(), state, callId, callerId);
    }

    /**
     * Notifies the Engine of an error related to a call
     *
     * @param  callId The unique identifier for the call associated with the error
     * @param  code The error type
     */
    final protected void callFailed(String callId, CallError code) {
        callFailed(getNativeRef(), callId, code, "");
    }

    /**
     * Notifies the Engine of an error related to a call
     *
     * @param  callId The unique identifier for the call associated with the error
     * @param  code The error type
     * @param  message A description of the error
     */
    final protected void callFailed(String callId, CallError code, String message) {
        callFailed(getNativeRef(), callId, code, message);
    }

    /**
     * Notifies the Engine that a caller id was received for an inbound call
     *
     * @param  callId The unique identifier for the call associated with the callId
     * @param  callerId The caller's identifier or phone number
     */
    final protected void callerIdReceived(String callId, String callerId) {
        callerIdReceived(getNativeRef(), callId, callerId);
    }

    /**
     * Notifies the Engine that sending the DTMF signal succeeded.
     *
     * @param  callId The unique identifier for the associated call
     *
     * @sa PhoneCallController::sendDTMF
     */
    final protected void sendDTMFSucceeded(String callId) {
        sendDTMFSucceeded(getNativeRef(), callId);
    }

    /**
     * Notifies the Engine that the DTMF signal could not be delivered to the remote party
     *
     * @param  callId The unique identifier for the associated call
     * @param  code The error type
     *
     * @sa PhoneCallController::sendDTMF
     */
    final protected void sendDTMFFailed(String callId, DTMFError code) {
        sendDTMFFailed(getNativeRef(), callId, code, "");
    }

    /**
     * Notifies the Engine that the DTMF signal could not be delivered to the remote party
     *
     * @param  callId The unique identifier for the associated call
     * @param  code The error type
     * @param  message A description of the error
     *
     * @sa PhoneCallController::sendDTMF
     */
    final protected void sendDTMFFailed(String callId, DTMFError code, String message) {
        sendDTMFFailed(getNativeRef(), callId, code, message);
    }

    /**
     * Notifies the Engine of the calling feature configuration of the connected calling device.
     * The configuration data may change if, for example, the connection mechanism to the calling device changes.
     * The provided configuration will override the default or previous configuration.
     *
     * See @c PhoneCallController::CallingDeviceConfigurationProperty for a
     * description of each configurable feature
     *
     * @param configurationMap A map of configuration properties to the boolean state of the properties
     */
    final protected void deviceConfigurationUpdated(
            HashMap<CallingDeviceConfigurationProperty, Boolean> configurationMap) {
        CallingDeviceConfigurationProperty[] configurations =
                new CallingDeviceConfigurationProperty[configurationMap.size()];
        boolean[] configurationValues = new boolean[configurationMap.size()];

        int index = 0;
        for (CallingDeviceConfigurationProperty key : configurationMap.keySet()) {
            configurations[index] = key;
            configurationValues[index] = configurationMap.get(key);
            index++;
        }
        deviceConfigurationUpdated(getNativeRef(), configurations, configurationValues);
    }

    /**
     * Generates a unique identifier for a call
     */
    final protected String createCallId() {
        return createCallId(getNativeRef());
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native void connectionStateChanged(long nativeRef, ConnectionState state);
    private native void callStateChanged(long nativeRef, CallState state, String callId, String callerId);
    private native void callFailed(long nativeRef, String callId, CallError code, String message);
    private native void callerIdReceived(long nativeRef, String callId, String callerId);
    private native void sendDTMFSucceeded(long nativeRef, String callId);
    private native void sendDTMFFailed(long nativeRef, String callId, DTMFError code, String message);
    private native void deviceConfigurationUpdated(
            long nativeRef, CallingDeviceConfigurationProperty[] configurations, boolean[] configurationValues);
    private native String createCallId(long nativeRef);
}

// END OF FILE
