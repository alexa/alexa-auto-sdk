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

#ifndef AACE_ALEXA_ALEXA_CLIENT_H
#define AACE_ALEXA_ALEXA_CLIENT_H

#include "AACE/Core/PlatformInterface.h"
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * The @c AlexaClient class should be extended by the platform implementation to handle standard AVS state changes in the
 * application.
 */
class AlexaClient : public aace::core::PlatformInterface {
protected:
    AlexaClient() = default;

public:
    virtual ~AlexaClient() = default;

    /**
     * The enum DialogState describes the state of the Alexa dialog interaction.
     */
    enum class DialogState {
        /**
         * Alexa is idle and ready for an interaction.
         */
        IDLE,
        /**
         * Alexa is currently listening.
         */
        LISTENING,
        /**
         * A customer request has been completed and no more input is accepted. In this state, Alexa is waiting for a
         * response from AVS.
         */
        THINKING,
        /**
         * Alexa is responding to a request with speech.
         */
        SPEAKING
    };

    /**
     * Called when the platform implementation should handle AVS UX dialog state changes.
     *
     * @param [in] state The new dialog-specific AVS UX state.
     * @sa DialogState
     */
    virtual void dialogStateChanged( DialogState state ) {}

    /**
     * @sa aace::alexa::AuthProviderEngineInterface::AuthState
     */
    using AuthState = aace::alexa::AuthProviderEngineInterface::AuthState;

    /**
     * @sa aace::alexa::AuthProviderEngineInterface::AuthError
     */
    using AuthError = aace::alexa::AuthProviderEngineInterface::AuthError;

    /**
     * Called when the platform implementation should handle AVS auth state changes.
     *
     * @param [in] state The new auth state.
     * @param [in] error The auth error if an error occurred.
     * @sa AuthState AuthError
     */
    virtual void authStateChanged( AuthState state, AuthError error ) {}

    /**
     * The enum ConnectionStatus describes the state of ACL connection.
     */
    enum class ConnectionStatus {
        /**
         * ACL is not connected to AVS.
         */
        DISCONNECTED,
        /**
         * ACL is attempting to establish a connection to AVS.
         */
        PENDING,
        /**
         * ACL is connected to AVS.
         */
        CONNECTED
    };

    /**
     * The enum ConnectionChangedReason encodes the reasons for state changes or failures in the connection attempt to the public API.
     */
    enum class ConnectionChangedReason {
        /**
         * The connection status changed due to the client interacting with the Connection public API.
         */
        ACL_CLIENT_REQUEST,
        /**
         * The connection attempt failed due to the Connection object being disabled.
         */
        ACL_DISABLED,
        /**
         * The connection attempt failed due to DNS resolution timeout.
         */
        DNS_TIMEDOUT,
        /**
         * The connection attempt failed due to timeout.
         */
        CONNECTION_TIMEDOUT,
        /**
         * The connection attempt failed due to excessive load on the server.
         */
        CONNECTION_THROTTLED,
        /**
         * The access credentials provided to ACL were invalid.
         */
        INVALID_AUTH,
        /**
         * There was a timeout sending a ping request.
         */
        PING_TIMEDOUT,
        /**
         * There was a timeout writing to AVS.
         */
        WRITE_TIMEDOUT,
        /**
         * There was a timeout reading from AVS.
         */
        READ_TIMEDOUT,
        /**
         * There was an underlying protocol error.
         */
        FAILURE_PROTOCOL_ERROR,
        /**
         * There was an internal error within ACL.
         */
        INTERNAL_ERROR,
        /**
         * There was an internal error on the server.
         */
        SERVER_INTERNAL_ERROR,
        /**
         * The server asked the client to reconnect.
         */
        SERVER_SIDE_DISCONNECT,
        /**
         * The server endpoint has changed.
         */
        SERVER_ENDPOINT_CHANGED
    };

    /**
     * Called when the platform implementation should handle AVS connection status changes.
     *
     * @param [in] status The new connection status.
     * @param [in] reason The reason the status change occurred.
     * @sa ConnectionStatus ConnectionChangedReason
     */
    virtual void connectionStatusChanged( ConnectionStatus status, ConnectionChangedReason reason ) {}
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_ALEXA_CLIENT_H
