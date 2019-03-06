/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/alexa/AlexaClientBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_ALEXA_CLIENT_BINDER_H
#define AACE_ALEXA_ALEXA_CLIENT_BINDER_H

#include "AACE/Alexa/AlexaClient.h"
#include "aace/core/PlatformInterfaceBinder.h"

class AlexaClientBinder : public PlatformInterfaceBinder, public aace::alexa::AlexaClient {
public:
    AlexaClientBinder() = default;

public:
    void dialogStateChanged( aace::alexa::AlexaClient::DialogState state ) override;
    void authStateChanged( aace::alexa::AlexaClient::AuthState state, aace::alexa::AlexaClient::AuthError error ) override;
    void connectionStatusChanged( aace::alexa::AlexaClient::ConnectionStatus status, aace::alexa::AlexaClient::ConnectionChangedReason reason ) override;

protected:
    void initialize( JNIEnv* env ) override;

private:
    jobject convert( aace::alexa::AlexaClient::DialogState state );
    jobject convert( aace::alexa::AuthProviderEngineInterface::AuthState state );
    jobject convert( aace::alexa::AuthProviderEngineInterface::AuthError error );
    jobject convert( aace::alexa::AlexaClient::ConnectionStatus status );
    jobject convert( aace::alexa::AlexaClient::ConnectionChangedReason reason );

private:
    jmethodID m_javaMethod_dialogStateChanged_state = nullptr;
    jmethodID m_javaMethod_authStateChanged_state_error = nullptr;
    jmethodID m_javaMethod_connectionStatusChanged_status_reason = nullptr;

    // DialogState
    ObjectRef m_enum_DialogState_IDLE;
    ObjectRef m_enum_DialogState_LISTENING;
    ObjectRef m_enum_DialogState_EXPECTING;
    ObjectRef m_enum_DialogState_THINKING;
    ObjectRef m_enum_DialogState_SPEAKING;

    // AuthState
    ObjectRef m_enum_AuthState_UNINITIALIZED;
    ObjectRef m_enum_AuthState_REFRESHED;
    ObjectRef m_enum_AuthState_EXPIRED;
    ObjectRef m_enum_AuthState_UNRECOVERABLE_ERROR;

    // AuthError
    ObjectRef m_enum_AuthError_NO_ERROR;
    ObjectRef m_enum_AuthError_UNKNOWN_ERROR;
    ObjectRef m_enum_AuthError_AUTHORIZATION_FAILED;
    ObjectRef m_enum_AuthError_UNAUTHORIZED_CLIENT;
    ObjectRef m_enum_AuthError_SERVER_ERROR;
    ObjectRef m_enum_AuthError_INVALID_REQUEST;
    ObjectRef m_enum_AuthError_INVALID_VALUE;
    ObjectRef m_enum_AuthError_AUTHORIZATION_EXPIRED;
    ObjectRef m_enum_AuthError_UNSUPPORTED_GRANT_TYPE;
    ObjectRef m_enum_AuthError_INVALID_CODE_PAIR;
    ObjectRef m_enum_AuthError_AUTHORIZATION_PENDING;
    ObjectRef m_enum_AuthError_SLOW_DOWN;
    ObjectRef m_enum_AuthError_INTERNAL_ERROR;
    ObjectRef m_enum_AuthError_INVALID_CBL_CLIENT_ID;

    // ConnectionStatus
    ObjectRef m_enum_ConnectionStatus_DISCONNECTED;
    ObjectRef m_enum_ConnectionStatus_PENDING;
    ObjectRef m_enum_ConnectionStatus_CONNECTED;

    // ConnectionChangedReason
    ObjectRef m_enum_ConnectionChangedReason_NONE;
    ObjectRef m_enum_ConnectionChangedReason_SUCCESS;
    ObjectRef m_enum_ConnectionChangedReason_UNRECOVERABLE_ERROR;
    ObjectRef m_enum_ConnectionChangedReason_ACL_CLIENT_REQUEST;
    ObjectRef m_enum_ConnectionChangedReason_ACL_DISABLED;
    ObjectRef m_enum_ConnectionChangedReason_DNS_TIMEDOUT;
    ObjectRef m_enum_ConnectionChangedReason_CONNECTION_TIMEDOUT;
    ObjectRef m_enum_ConnectionChangedReason_CONNECTION_THROTTLED;
    ObjectRef m_enum_ConnectionChangedReason_INVALID_AUTH;
    ObjectRef m_enum_ConnectionChangedReason_PING_TIMEDOUT;
    ObjectRef m_enum_ConnectionChangedReason_WRITE_TIMEDOUT;
    ObjectRef m_enum_ConnectionChangedReason_READ_TIMEDOUT;
    ObjectRef m_enum_ConnectionChangedReason_FAILURE_PROTOCOL_ERROR;
    ObjectRef m_enum_ConnectionChangedReason_INTERNAL_ERROR;
    ObjectRef m_enum_ConnectionChangedReason_SERVER_INTERNAL_ERROR;
    ObjectRef m_enum_ConnectionChangedReason_SERVER_SIDE_DISCONNECT;
    ObjectRef m_enum_ConnectionChangedReason_SERVER_ENDPOINT_CHANGED;
};

#endif //AACE_ALEXA_ALEXA_CLIENT_BINDER_H
