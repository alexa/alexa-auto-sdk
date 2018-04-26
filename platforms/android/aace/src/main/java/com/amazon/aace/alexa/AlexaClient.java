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

// aace/alexa/AlexaClient.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c AlexaClient class should be extended by the platform implementation to handle standard AVS state changes in the application.
 */
public class AlexaClient extends PlatformInterface
{
    /**
     * The enum DialogState describes the state of the Alexa dialog interaction.
     */
    public enum DialogState
    {
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
         * A customer request has been completed and no more input is accepted. In this state, Alexa is waiting for a response from AVS.
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
        private DialogState( String name ) {
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
     * The enum AuthState describes the state of authorization.
     */
    public enum AuthState
    {
        /**
         * Authorization not yet acquired.
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
         * Authorization failed in a manner that cannot be corrected by retrying.
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
        private AuthState( String name ) {
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
     * The enum AuthError encodes possible errors which may occur when changing authorization state.
     */
    public enum AuthError
    {
        /**
         * No error.
         * @hideinitializer
         */
        NO_ERROR("NO_ERROR"),
        /**
         * An unknown body containing no error field has been encountered.
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
         * The request is missing a required parameter, has an invalid value, or is otherwise improperly formed.
         * @hideinitializer
         */
        INVALID_REQUEST("INVALID_REQUEST"),
        /**
         * The authorization code is invalid, expired, revoked, or was issued to a different client.
         * @hideinitializer
         */
        AUTHORIZATION_EXPIRED("AUTHORIZATION_EXPIRED"),
        /**
         * The client specified the wrong token type.
         * @hideinitializer
         */
        UNSUPPORTED_GRANT_TYPE("UNSUPPORTED_GRANT_TYPE");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AuthError( String name ) {
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
     * The enum ConnectionStatus describes the state of ACL connection.
     */
    public enum ConnectionStatus
    {
        /**
         * ACL is not connected to AVS.
         * @hideinitializer
         */
        DISCONNECTED("DISCONNECTED"),
        /**
         * ACL is attempting to establish a connection to AVS.
         * @hideinitializer
         */
        PENDING("PENDING"),
        /**
         * ACL is connected to AVS.
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
        private ConnectionStatus( String name ) {
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
     * The enum ConnectionChangedReason encodes the reasons for state changes or failures in the connection attempt to the public API.
     */
    public enum ConnectionChangedReason
    {
        /**
         * The connection status changed due to the client interacting with the Connection public API.
         * @hideinitializer
         */
        ACL_CLIENT_REQUEST("ACL_CLIENT_REQUEST"),
        /**
         * The connection attempt failed due to the Connection object being disabled.
         * @hideinitializer
         */
        ACL_DISABLED("ACL_DISABLED"),
        /**
         * The connection attempt failed due to DNS resolution timeout.
         * @hideinitializer
         */
        DNS_TIMEDOUT("DNS_TIMEDOUT"),
        /**
         * The connection attempt failed due to timeout.
         * @hideinitializer
         */
        CONNECTION_TIMEDOUT("CONNECTION_TIMEDOUT"),
        /**
         * The connection attempt failed due to excessive load on the server.
         * @hideinitializer
         */
        CONNECTION_THROTTLED("CONNECTION_THROTTLED"),
        /**
         * The access credentials provided to ACL were invalid.
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
         * There was an internal error within ACL.
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
        private ConnectionChangedReason( String name ) {
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
     * Called when the platform implementation should handle AVS UX dialog state changes.
     *
     * @param [in] state The new dialog-specific AVS UX state.
     * @sa DialogState
     */
    public void dialogStateChanged( DialogState state ) {
    }

    /**
     * Called when the platform implementation should handle AVS auth state changes.
     *
     * @param [in] state The new auth state.
     * @param [in] error The auth error if an error occurred.
     * @sa AuthState AuthError
     */
    public void authStateChanged( AuthState state, AuthError error ) {
    }

    /**
     * Called when the platform implementation should handle AVS connection status changes.
     *
     * @param [in] status The new connection status.
     * @param [in] reason The reason the status change occurred.
     * @sa ConnectionStatus ConnectionChangedReason
     */
    public void connectionStatusChanged( ConnectionStatus status, ConnectionChangedReason reason ) {
    }
}

// END OF FILE
