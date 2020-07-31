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

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * AlexaClient should be extended to handle Alexa state changes on the platform.
 */
abstract public class AlexaClient extends PlatformInterface {
    /**
     * Describes the state of Alexa dialog interaction
     */
    public enum DialogState {
        /**
         * Alexa is idle and ready for an interaction.
         * @hideinitializer
         */
        IDLE("IDLE"),
        /**
         * Alexa is currently listening.
         * @hideinitializer
         */
        LISTENING("LISTENING"),
        /**
         * Alexa is currently expecting a response from the user.
         * @hideinitializer
         */
        EXPECTING("EXPECTING"),
        /**
         * A user request has completed, and no more user input is being accepted.
         * Alexa is waiting for a response from AVS.
         * @hideinitializer
         */
        THINKING("THINKING"),
        /**
         * Alexa is responding to a request with speech.
         * @hideinitializer
         */
        SPEAKING("SPEAKING");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private DialogState(String name) {
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
     * Describes the state of client authorization with AVS
     */
    public enum AuthState {
        /**
         * Authorization has not yet been acquired.
         * @hideinitializer
         */
        UNINITIALIZED("UNINITIALIZED"),
        /**
         * Authorization has been refreshed.
         * @hideinitializer
         */
        REFRESHED("REFRESHED"),
        /**
         * Authorization has expired.
         * @hideinitializer
         */
        EXPIRED("EXPIRED"),
        /**
         * Authorization has failed in a manner that cannot be corrected by retrying.
         * @hideinitializer
         */
        UNRECOVERABLE_ERROR("UNRECOVERABLE_ERROR");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AuthState(String name) {
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
     * Describes an error during an attempt to authorize with AVS
     */
    public enum AuthError {
        /**
         * No error encountered
         * @hideinitializer
         */
        NO_ERROR("NO_ERROR"),
        /**
         * An error was encountered, but no error description can be determined.
         * @hideinitializer
         */
        UNKNOWN_ERROR("UNKNOWN_ERROR"),
        /**
         * The client authorization failed.
         * @hideinitializer
         */
        AUTHORIZATION_FAILED("AUTHORIZATION_FAILED"),
        /**
         * The client is not authorized to use authorization codes.
         * @hideinitializer
         */
        UNAUTHORIZED_CLIENT("UNAUTHORIZED_CLIENT"),
        /**
         * The server encountered a runtime error.
         * @hideinitializer
         */
        SERVER_ERROR("SERVER_ERROR"),
        /**
         * The request is missing a required parameter, has an invalid value, or is otherwise malformed.
         * @hideinitializer
         */
        INVALID_REQUEST("INVALID_REQUEST"),
        /**
         * One of the values in the request was invalid.
         * @hideinitializer
         */
        INVALID_VALUE("INVALID_VALUE"),
        /**
         * The authorization code is invalid, expired, revoked, or was issued to a different client.
         * @hideinitializer
         */
        AUTHORIZATION_EXPIRED("AUTHORIZATION_EXPIRED"),
        /**
         * The client specified the wrong token type.
         * @hideinitializer
         */
        UNSUPPORTED_GRANT_TYPE("UNSUPPORTED_GRANT_TYPE"),
        /**
         * Invalid code pair provided in Code-based linking token request.
         * @hideinitializer
         */
        INVALID_CODE_PAIR("INVALID_CODE_PAIR"),
        /**
         * Waiting for user to authorize the specified code pair.
         * @hideinitializer
         */
        AUTHORIZATION_PENDING("AUTHORIZATION_PENDING"),
        /**
         * Client should slow down in the rate of requests polling for an access token.
         * @hideinitializer
         */
        SLOW_DOWN("SLOW_DOWN"),
        /**
         * Internal error in client code.
         * @hideinitializer
         */
        INTERNAL_ERROR("INTERNAL_ERROR"),
        /**
         * Client ID not valid for use with code based linking.
         * @hideinitializer
         */
        INVALID_CBL_CLIENT_ID("INVALID_CBL_CLIENT_ID");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AuthError(String name) {
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
     * Describes the status of an AVS connection
     */
    public enum ConnectionStatus {
        /**
         * Not connected to AVS
         * @hideinitializer
         */
        DISCONNECTED("DISCONNECTED"),
        /**
         * Attempting to establish a connection to AVS
         * @hideinitializer
         */
        PENDING("PENDING"),
        /**
         * Connected to AVS
         * @hideinitializer
         */
        CONNECTED("CONNECTED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ConnectionStatus(String name) {
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
     * Describes the reason for a change in AVS connection status
     */
    public enum ConnectionChangedReason {
        /**
         * No reason specified.
         * @hideinitializer
         */
        NONE("NONE"),
        /**
         * The connection status changed due to a successful operation.
         * @hideinitializer
         */
        SUCCESS("SUCCESS"),
        /**
         * The connection status changed due to an error from which there is no recovery.
         * @hideinitializer
         */
        UNRECOVERABLE_ERROR("UNRECOVERABLE_ERROR"),
        /**
         * The connection status changed due to a client request.
         * @hideinitializer
         */
        ACL_CLIENT_REQUEST("ACL_CLIENT_REQUEST"),
        /**
         * The connection attempt failed because connection was disabled.
         * @hideinitializer
         */
        ACL_DISABLED("ACL_DISABLED"),
        /**
         * The connection attempt failed due to a DNS resolution timeout.
         * @hideinitializer
         */
        DNS_TIMEDOUT("DNS_TIMEDOUT"),
        /**
         * The connection attempt failed due to a connection timeout.
         * @hideinitializer
         */
        CONNECTION_TIMEDOUT("CONNECTION_TIMEDOUT"),
        /**
         * The connection attempt failed due to excessive load on the server.
         * @hideinitializer
         */
        CONNECTION_THROTTLED("CONNECTION_THROTTLED"),
        /**
         * The provided access credentials were invalid.
         * @hideinitializer
         */
        INVALID_AUTH("INVALID_AUTH"),
        /**
         * There was a timeout sending a ping request.
         * @hideinitializer
         */
        PING_TIMEDOUT("PING_TIMEDOUT"),
        /**
         * There was a timeout writing to AVS.
         * @hideinitializer
         */
        WRITE_TIMEDOUT("WRITE_TIMEDOUT"),
        /**
         * There was a timeout reading from AVS.
         * @hideinitializer
         */
        READ_TIMEDOUT("READ_TIMEDOUT"),
        /**
         * There was an underlying protocol error.
         * @hideinitializer
         */
        FAILURE_PROTOCOL_ERROR("FAILURE_PROTOCOL_ERROR"),
        /**
         * There was an internal error.
         * @hideinitializer
         */
        INTERNAL_ERROR("INTERNAL_ERROR"),
        /**
         * There was an internal error on the server.
         * @hideinitializer
         */
        SERVER_INTERNAL_ERROR("SERVER_INTERNAL_ERROR"),
        /**
         * The server asked the client to reconnect.
         * @hideinitializer
         */
        SERVER_SIDE_DISCONNECT("SERVER_SIDE_DISCONNECT"),
        /**
         * The server endpoint has changed.
         * @hideinitializer
         */
        SERVER_ENDPOINT_CHANGED("SERVER_ENDPOINT_CHANGED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ConnectionChangedReason(String name) {
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
     * Notifies the platform implementation of an Alexa dialog state change
     *
     * @param  state The new Alexa dialog state
     *
     * @note It is the responsibility of the platform implementation to provide a familiar Alexa experience for the
     * user. See the AVS UX Attention System guidelines for recommendations on communicating Alexa attention states:
     * https://developer.amazon.com/docs/alexa-voice-service/ux-design-attention.html#implement
     */
    public void dialogStateChanged(DialogState state) {}

    /**
     * Notifies the platform implementation of an AVS authorization state change
     *
     * @param  state The new authorization state
     *
     * @param  error The error state of the authorization attempt
     */
    public void authStateChanged(AuthState state, AuthError error) {}

    /**
     * Notifies the platform implementation of an AVS connection status change
     *
     * @param  status The new AVS connection status
     *
     * @param  reason The reason for the status change
     */
    public void connectionStatusChanged(ConnectionStatus status, ConnectionChangedReason reason) {}

    /**
     * Stops the foreground activity if there is one. This acts as a "stop" button that can be used to stop
     * an ongoing activity that has acquired the audio or visual focus. This call will trigger the action
     * to stop all user-observable activities and return immediately
     */
    public void stopForegroundActivity() {
        stopForegroundActivity(getNativeRef());
    }

    private native long stopForegroundActivity(long nativeObject);

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
}
