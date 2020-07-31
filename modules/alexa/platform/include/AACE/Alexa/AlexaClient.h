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

#ifndef AACE_ALEXA_ALEXA_CLIENT_H
#define AACE_ALEXA_ALEXA_CLIENT_H

#include "AACE/Core/PlatformInterface.h"
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * AlexaClient should be extended to handle Alexa state changes on the platform.
 */
class AlexaClient : public aace::core::PlatformInterface {
protected:
    AlexaClient() = default;

public:
    virtual ~AlexaClient();

    /**
     * Describes the state of Alexa dialog interaction
     */
    enum class DialogState {

        /**
         * Alexa is idle and ready for an interaction.
         */
        IDLE,

        /**
         * Alexa is listening.
         */
        LISTENING,

        /**
         * Alexa is expecting a response from the user.
         */
        EXPECTING,

        /**
         * A user request has completed, and no more user input is being accepted.
         * Alexa is waiting for a response from AVS.
         */
        THINKING,

        /**
         * Alexa is responding to a request with speech.
         */
        SPEAKING
    };

    /**
     * Notifies the platform implementation of an Alexa dialog state change
     *
     * @param [in] state The new Alexa dialog state
     *
     * @note The platform implementation is responsible for providing a familiar Alexa experience for the user.
     * See the AVS UX Attention System guidelines for recommendations on communicating 
     * Alexa attention states: https://developer.amazon.com/docs/alexa-voice-service/ux-design-attention.html#implement
     */
    virtual void dialogStateChanged(DialogState state) {
    }

    using AuthState = aace::alexa::AuthProviderEngineInterface::AuthState;
    using AuthError = aace::alexa::AuthProviderEngineInterface::AuthError;

    /**
     * Notifies the platform implementation of an AVS authorization state change
     *
     * @param [in] state The new authorization state
     * @param [in] error The error state of the authorization attempt
     */
    virtual void authStateChanged(AuthState state, AuthError error) {
    }

    /**
     * Describes the AVS connection status
     */
    enum class ConnectionStatus {

        /**
         * Not connected to AVS
         */
        DISCONNECTED,

        /**
         * Attempting to establish a connection to AVS
         */
        PENDING,

        /**
         * Connected to AVS
         */
        CONNECTED
    };

    /**
     * Describes the reason for a change in AVS connection status
     */
    enum class ConnectionChangedReason {

        /**
         * No reason specified.
         */
        NONE,

        /**
         * The connection status changed due to a successful operation.
         */
        SUCCESS,

        /**
         * The connection status changed due to an error from which there is no recovery.
         */
        UNRECOVERABLE_ERROR,

        /**
         * The connection status changed due to a client request.
         */
        ACL_CLIENT_REQUEST,

        /**
         * The connection attempt failed because connection was disabled.
         */
        ACL_DISABLED,

        /**
         * The connection attempt failed due to a DNS resolution timeout.
         */
        DNS_TIMEDOUT,

        /**
         * The connection attempt failed due to a connection timeout.
         */
        CONNECTION_TIMEDOUT,

        /**
         * The connection attempt failed due to excessive load on the server.
         */
        CONNECTION_THROTTLED,

        /**
         * The provided access credentials were invalid.
         */
        INVALID_AUTH,

        /**
         * A ping request timed out.
         */
        PING_TIMEDOUT,

        /**
         * A write request to AVS timed out.
         */
        WRITE_TIMEDOUT,

        /**
         * A read request from AVS timed out.
         */
        READ_TIMEDOUT,

        /**
         * There was an underlying protocol error.
         */
        FAILURE_PROTOCOL_ERROR,

        /**
         * There was an internal error.
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
     * Notifies the platform implementation of an AVS connection status change
     *
     * @param [in] status The new AVS connection status
     * @param [in] reason The reason for the AVS connection status change
     */
    virtual void connectionStatusChanged(ConnectionStatus status, ConnectionChangedReason reason) {
    }

    /**
     * Stops the foreground activity if there is one. This acts as a "stop" button that you use to stop
     * an ongoing activity that has acquired the audio or visual focus. This call stops all user-observable
     * activities and returns immediately.
     */
    void stopForegroundActivity();

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * This is for the use of Auto SDK. Do not call this from the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<aace::alexa::AlexaClientEngineInterface> alexaClientEngineInterface);

private:
    std::weak_ptr<aace::alexa::AlexaClientEngineInterface> m_alexaClientEngineInterface;
};

inline std::ostream& operator<<(std::ostream& stream, const AlexaClient::DialogState& state) {
    switch (state) {
        case AlexaClient::DialogState::IDLE:
            stream << "IDLE";
            break;
        case AlexaClient::DialogState::LISTENING:
            stream << "LISTENING";
            break;
        case AlexaClient::DialogState::EXPECTING:
            stream << "EXPECTING";
            break;
        case AlexaClient::DialogState::THINKING:
            stream << "THINKING";
            break;
        case AlexaClient::DialogState::SPEAKING:
            stream << "SPEAKING";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const AlexaClient::ConnectionStatus& status) {
    switch (status) {
        case AlexaClient::ConnectionStatus::DISCONNECTED:
            stream << "DISCONNECTED";
            break;
        case AlexaClient::ConnectionStatus::PENDING:
            stream << "PENDING";
            break;
        case AlexaClient::ConnectionStatus::CONNECTED:
            stream << "CONNECTED";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const AlexaClient::ConnectionChangedReason& reason) {
    switch (reason) {
        case AlexaClient::ConnectionChangedReason::NONE:
            stream << "NONE";
            break;
        case AlexaClient::ConnectionChangedReason::SUCCESS:
            stream << "SUCCESS";
            break;
        case AlexaClient::ConnectionChangedReason::UNRECOVERABLE_ERROR:
            stream << "UNRECOVERABLE_ERROR";
            break;
        case AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST:
            stream << "ACL_CLIENT_REQUEST";
            break;
        case AlexaClient::ConnectionChangedReason::ACL_DISABLED:
            stream << "ACL_DISABLED";
            break;
        case AlexaClient::ConnectionChangedReason::DNS_TIMEDOUT:
            stream << "DNS_TIMEDOUT";
            break;
        case AlexaClient::ConnectionChangedReason::CONNECTION_TIMEDOUT:
            stream << "CONNECTION_TIMEDOUT";
            break;
        case AlexaClient::ConnectionChangedReason::CONNECTION_THROTTLED:
            stream << "CONNECTION_THROTTLED";
            break;
        case AlexaClient::ConnectionChangedReason::INVALID_AUTH:
            stream << "INVALID_AUTH";
            break;
        case AlexaClient::ConnectionChangedReason::PING_TIMEDOUT:
            stream << "PING_TIMEDOUT";
            break;
        case AlexaClient::ConnectionChangedReason::WRITE_TIMEDOUT:
            stream << "WRITE_TIMEDOUT";
            break;
        case AlexaClient::ConnectionChangedReason::READ_TIMEDOUT:
            stream << "READ_TIMEDOUT";
            break;
        case AlexaClient::ConnectionChangedReason::FAILURE_PROTOCOL_ERROR:
            stream << "FAILURE_PROTOCOL_ERROR";
            break;
        case AlexaClient::ConnectionChangedReason::INTERNAL_ERROR:
            stream << "INTERNAL_ERROR";
            break;
        case AlexaClient::ConnectionChangedReason::SERVER_INTERNAL_ERROR:
            stream << "SERVER_INTERNAL_ERROR";
            break;
        case AlexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT:
            stream << "SERVER_SIDE_DISCONNECT";
            break;
        case AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED:
            stream << "SERVER_ENDPOINT_CHANGED";
            break;
    }
    return stream;
}

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_ALEXA_CLIENT_H
