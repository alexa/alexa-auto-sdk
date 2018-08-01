/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

/**
 * PhoneCallController should be extended to handle directives to initiate a phone call
 * on a calling device (e.g. mobile phone). It also provides interfaces for notifying the Engine
 * of the state of a call session and the state of platform connection to the calling device.
 * The platform implementation is responsible for managing the lifecycle and user experience of a call session
 * and connection to the calling device.
 */
abstract public class PhoneCallController extends PlatformInterface
{
    /**
     * Specifies the state of connection to the platform calling device
     */
    public enum ConnectionState
    {
        /**
         * The platform is connected to a calling device
         * @hideinitializer
         */
        CONNECTED("CONNECTED"),
        /**
         * The platform is not connected to a calling device
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
        private ConnectionState( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public PhoneCallController() {
    }

    /**
     * Notifies the platform implementation to initiate an outgoing phone call to a destination address
     *
     * @param  payload Details of the outgoing call in structured JSON format. See the following
     * payload structure and the description of each field:
     *
     * @code    {.json})
     *  {
     *     "callId": "{{STRING}}",
     *     "callee": {
     *       "details": "{{STRING}}",
     *       "defaultAddress": {
     *         "protocol": "{{STRING}}",
     *         "format": "{{STRING}}",
     *         "value": "{{STRING}}"
     *       },
     *       "alternativeAddresses": [{
     *         "protocol": "{{STRING}}",
     *         "format": "{{STRING}}",
     *         "value": {{STRING}}
     *       }]
     *     }
     *   }
     * }
     * @endcode
     *
     *
     * @li callId (required): A unique identifier for the call
     *
     * @li callee (required): The destination of the outgoing call
     *
     * @li callee.details (optional): Descriptive information about the callee
     *
     * @li callee.defaultAddress (required): The default address to use for calling the callee
     *
     * @li callee.alternativeAddresses (optional): An array of alternate addresses for the existing callee
     *
     * @li address.protocol (required): The protocol for this address of the callee (e.g. PSTN, SIP, H323, etc.)
     *
     * @li address.format (optional): The format for this address of the callee (e.g. E.164, E.163, E.123, DIN5008, etc.)
     *
     * @li address.value (required): The address of the callee.
     *
     * @return @c true if the platform implementation will initiate the call,
     * else @c false
     */
    public boolean dial( String payload ) {
        return false;
    }

    /**
     * Notifies the Engine that a phone call was activated on a calling device.
     * @c callActivated() should be called in response to a @c dial() directive in which the platform implementation returned @c true. @c callId must match the @c callId from the @c dial() payload.
     * When the platform implementation calls @c callActivated() for a call initiated outside of the scope of Alexa,
     * it is responsible for creating a UUID for this call session.
     *
     * @param  callId The unique identifier for the call
     */
    final protected void callActivated( String callId ) {
        callActivated( getNativeObject(), callId );
    }

    /**
     * Notifies the Engine that an active call was terminated or an ongoing phone call setup was cancelled
     *
     * @param  callId The unique identifier for the call
     */
    final protected void callTerminated( String callId ) {
        callTerminated( getNativeObject(), callId );
    }

    /**
     * Notifies the Engine of an error in initiating or maintaining a call on the platform calling device
     *
     * @param  callId The unique identifier for the call
     *
     * @param  error An error status code:
     *
     * @li 4xx range: Validation failure for the input from the @c dial() directive
     *
     * @li 500: Internal error on the platform unrelated to the cellular network
     *
     * @li 503: Error on the platform related to the cellular network
     */
    final protected void callFailed( String callId, String error ) {
        callFailed( getNativeObject(), callId, error );
    }

    /**
     * Notifies the Engine of an error in initiating or maintaining a call on a calling device
     *
     * @param  callId The unique identifier for the call
     *
     * @param  error An error status code:
     *
     * @li 4xx range: Validation failure for the input from the @c dial() directive
     *
     * @li 500: Internal error on the platform unrelated to the cellular network
     *
     * @li 503: Error on the platform related to the cellular network
     *
     * @param  message A description of the error
     */
    final protected void callFailed( String callId, String error, String message ) {
        callFailed( getNativeObject(), callId, error, message );
    }

    /**
     * Notifies the Engine of a change in connection state of a calling device.
     */
    public void connectionStateChanged( ConnectionState state ) {
        connectionStateChanged( getNativeObject(), state );
    }

    private native void callActivated( long nativeObject, String callId );
    private native void callTerminated( long nativeObject, String callId );
    private native void callFailed( long nativeObject, String callId, String error );
    private native void callFailed( long nativeObject, String callId, String error, String message );
    private native void connectionStateChanged( long nativeObject, ConnectionState state );

}

// END OF FILE
