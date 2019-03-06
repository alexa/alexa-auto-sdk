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

// aace/alexa/AlexaClientBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/AlexaClientBinder.h"

void AlexaClientBinder::initialize( JNIEnv* env )
{
    //jclass javaClass = env->GetObjectClass( getJavaObject() );
    m_javaMethod_dialogStateChanged_state = env->GetMethodID( getJavaClass(), "dialogStateChanged", "(Lcom/amazon/aace/alexa/AlexaClient$DialogState;)V" );
    m_javaMethod_authStateChanged_state_error = env->GetMethodID( getJavaClass(), "authStateChanged", "(Lcom/amazon/aace/alexa/AlexaClient$AuthState;Lcom/amazon/aace/alexa/AlexaClient$AuthError;)V" );
    m_javaMethod_connectionStatusChanged_status_reason = env->GetMethodID( getJavaClass(), "connectionStatusChanged", "(Lcom/amazon/aace/alexa/AlexaClient$ConnectionStatus;Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;)V" );

    // DialogState
    jclass dialogStateEnumClass = env->FindClass( "com/amazon/aace/alexa/AlexaClient$DialogState" );
    m_enum_DialogState_IDLE = NativeLib::FindEnum( env, dialogStateEnumClass, "IDLE", "Lcom/amazon/aace/alexa/AlexaClient$DialogState;" );
    m_enum_DialogState_LISTENING = NativeLib::FindEnum( env, dialogStateEnumClass, "LISTENING", "Lcom/amazon/aace/alexa/AlexaClient$DialogState;" );
    m_enum_DialogState_EXPECTING = NativeLib::FindEnum( env, dialogStateEnumClass, "EXPECTING", "Lcom/amazon/aace/alexa/AlexaClient$DialogState;" );
    m_enum_DialogState_THINKING = NativeLib::FindEnum( env, dialogStateEnumClass, "THINKING", "Lcom/amazon/aace/alexa/AlexaClient$DialogState;" );
    m_enum_DialogState_SPEAKING = NativeLib::FindEnum( env, dialogStateEnumClass, "SPEAKING", "Lcom/amazon/aace/alexa/AlexaClient$DialogState;" );

    // AuthState
    jclass authStateEnumClass = env->FindClass( "com/amazon/aace/alexa/AlexaClient$AuthState" );
    m_enum_AuthState_UNINITIALIZED = NativeLib::FindEnum( env, authStateEnumClass, "UNINITIALIZED", "Lcom/amazon/aace/alexa/AlexaClient$AuthState;" );
    m_enum_AuthState_REFRESHED = NativeLib::FindEnum( env, authStateEnumClass, "REFRESHED", "Lcom/amazon/aace/alexa/AlexaClient$AuthState;" );
    m_enum_AuthState_EXPIRED = NativeLib::FindEnum( env, authStateEnumClass, "EXPIRED", "Lcom/amazon/aace/alexa/AlexaClient$AuthState;" );
    m_enum_AuthState_UNRECOVERABLE_ERROR = NativeLib::FindEnum( env, authStateEnumClass, "UNRECOVERABLE_ERROR", "Lcom/amazon/aace/alexa/AlexaClient$AuthState;" );

    // AuthError
    jclass authErrorEnumClass = env->FindClass( "com/amazon/aace/alexa/AlexaClient$AuthError" );
    m_enum_AuthError_NO_ERROR = NativeLib::FindEnum( env, authErrorEnumClass, "NO_ERROR", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_UNKNOWN_ERROR = NativeLib::FindEnum( env, authErrorEnumClass, "UNKNOWN_ERROR", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_AUTHORIZATION_FAILED = NativeLib::FindEnum( env, authErrorEnumClass, "AUTHORIZATION_FAILED", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_UNAUTHORIZED_CLIENT = NativeLib::FindEnum( env, authErrorEnumClass, "UNAUTHORIZED_CLIENT", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_SERVER_ERROR = NativeLib::FindEnum( env, authErrorEnumClass, "SERVER_ERROR", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_INVALID_REQUEST = NativeLib::FindEnum( env, authErrorEnumClass, "INVALID_REQUEST", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_INVALID_VALUE = NativeLib::FindEnum( env, authErrorEnumClass, "INVALID_VALUE", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_AUTHORIZATION_EXPIRED = NativeLib::FindEnum( env, authErrorEnumClass, "AUTHORIZATION_EXPIRED", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_UNSUPPORTED_GRANT_TYPE = NativeLib::FindEnum( env, authErrorEnumClass, "UNSUPPORTED_GRANT_TYPE", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_INVALID_CODE_PAIR = NativeLib::FindEnum( env, authErrorEnumClass, "INVALID_CODE_PAIR", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_AUTHORIZATION_PENDING = NativeLib::FindEnum( env, authErrorEnumClass, "AUTHORIZATION_PENDING", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_SLOW_DOWN = NativeLib::FindEnum( env, authErrorEnumClass, "SLOW_DOWN", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_INTERNAL_ERROR = NativeLib::FindEnum( env, authErrorEnumClass, "INTERNAL_ERROR", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );
    m_enum_AuthError_INVALID_CBL_CLIENT_ID = NativeLib::FindEnum( env, authErrorEnumClass, "INVALID_CBL_CLIENT_ID", "Lcom/amazon/aace/alexa/AlexaClient$AuthError;" );

    // ConnectionStatus
    jclass connectionStatusEnumClass = env->FindClass( "com/amazon/aace/alexa/AlexaClient$ConnectionStatus" );
    m_enum_ConnectionStatus_DISCONNECTED = NativeLib::FindEnum( env, connectionStatusEnumClass, "DISCONNECTED", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionStatus;" );
    m_enum_ConnectionStatus_PENDING = NativeLib::FindEnum( env, connectionStatusEnumClass, "PENDING", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionStatus;" );
    m_enum_ConnectionStatus_CONNECTED = NativeLib::FindEnum( env, connectionStatusEnumClass, "CONNECTED", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionStatus;" );

    // ConnectionChangedReason
    jclass connectionChangedReasonEnumClass = env->FindClass( "com/amazon/aace/alexa/AlexaClient$ConnectionChangedReason" );
    m_enum_ConnectionChangedReason_NONE = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "NONE", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_SUCCESS = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "SUCCESS", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_UNRECOVERABLE_ERROR = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "UNRECOVERABLE_ERROR", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_ACL_CLIENT_REQUEST = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "ACL_CLIENT_REQUEST", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_ACL_DISABLED = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "ACL_DISABLED", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_DNS_TIMEDOUT = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "DNS_TIMEDOUT", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_CONNECTION_TIMEDOUT = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "CONNECTION_TIMEDOUT", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_CONNECTION_THROTTLED = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "CONNECTION_THROTTLED", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_INVALID_AUTH = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "INVALID_AUTH", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_PING_TIMEDOUT = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "PING_TIMEDOUT", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_WRITE_TIMEDOUT = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "WRITE_TIMEDOUT", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_READ_TIMEDOUT = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "READ_TIMEDOUT", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_FAILURE_PROTOCOL_ERROR = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "FAILURE_PROTOCOL_ERROR", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_INTERNAL_ERROR = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "INTERNAL_ERROR", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_SERVER_INTERNAL_ERROR = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "SERVER_INTERNAL_ERROR", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_SERVER_SIDE_DISCONNECT = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "SERVER_SIDE_DISCONNECT", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
    m_enum_ConnectionChangedReason_SERVER_ENDPOINT_CHANGED = NativeLib::FindEnum( env, connectionChangedReasonEnumClass, "SERVER_ENDPOINT_CHANGED", "Lcom/amazon/aace/alexa/AlexaClient$ConnectionChangedReason;" );
}

void AlexaClientBinder::dialogStateChanged( aace::alexa::AlexaClient::DialogState state )
{
    if( getJavaObject() != nullptr && m_javaMethod_dialogStateChanged_state != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_dialogStateChanged_state, convert( state ) );
        }
    }
}

void AlexaClientBinder::authStateChanged( aace::alexa::AlexaClient::AuthState state, aace::alexa::AlexaClient::AuthError error )
{
    if( getJavaObject() != nullptr && m_javaMethod_authStateChanged_state_error != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_authStateChanged_state_error, convert( state ), convert( error ) );
        }
    }
}

void AlexaClientBinder::connectionStatusChanged( aace::alexa::AlexaClient::ConnectionStatus status, aace::alexa::AlexaClient::ConnectionChangedReason reason )
{
    if( getJavaObject() != nullptr && m_javaMethod_connectionStatusChanged_status_reason != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_connectionStatusChanged_status_reason, convert( status ), convert( reason ) );
        }
    }
}

jobject AlexaClientBinder::convert( aace::alexa::AlexaClient::DialogState state )
{
    switch( state )
    {
        case aace::alexa::AlexaClient::DialogState::IDLE:
            return m_enum_DialogState_IDLE.get();
        case aace::alexa::AlexaClient::DialogState::LISTENING:
            return m_enum_DialogState_LISTENING.get();
        case aace::alexa::AlexaClient::DialogState::EXPECTING:
            return m_enum_DialogState_EXPECTING.get();
        case aace::alexa::AlexaClient::DialogState::THINKING:
            return m_enum_DialogState_THINKING.get();
        case aace::alexa::AlexaClient::DialogState::SPEAKING:
            return m_enum_DialogState_SPEAKING.get();
    }
}

jobject AlexaClientBinder::convert( aace::alexa::AuthProviderEngineInterface::AuthState state )
{
    switch( state )
    {
        case aace::alexa::AuthProviderEngineInterface::AuthState::UNINITIALIZED:
            return m_enum_AuthState_UNINITIALIZED.get();
        case aace::alexa::AuthProviderEngineInterface::AuthState::REFRESHED:
            return m_enum_AuthState_REFRESHED.get();
        case aace::alexa::AuthProviderEngineInterface::AuthState::EXPIRED:
            return m_enum_AuthState_EXPIRED.get();
        case aace::alexa::AuthProviderEngineInterface::AuthState::UNRECOVERABLE_ERROR:
            return m_enum_AuthState_UNRECOVERABLE_ERROR.get();
    }
}

jobject AlexaClientBinder::convert( aace::alexa::AuthProviderEngineInterface::AuthError error )
{
    switch( error )
    {
        case aace::alexa::AuthProviderEngineInterface::AuthError::NO_ERROR:
            return m_enum_AuthError_NO_ERROR.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::UNKNOWN_ERROR:
            return m_enum_AuthError_UNKNOWN_ERROR.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::AUTHORIZATION_FAILED:
            return m_enum_AuthError_AUTHORIZATION_FAILED.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::UNAUTHORIZED_CLIENT:
            return m_enum_AuthError_UNAUTHORIZED_CLIENT.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::SERVER_ERROR:
            return m_enum_AuthError_SERVER_ERROR.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::INVALID_REQUEST:
            return m_enum_AuthError_INVALID_REQUEST.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::INVALID_VALUE:
            return m_enum_AuthError_INVALID_VALUE.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::AUTHORIZATION_EXPIRED:
            return m_enum_AuthError_AUTHORIZATION_EXPIRED.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::UNSUPPORTED_GRANT_TYPE:
            return m_enum_AuthError_UNSUPPORTED_GRANT_TYPE.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::INVALID_CODE_PAIR:
            return m_enum_AuthError_INVALID_CODE_PAIR.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::AUTHORIZATION_PENDING:
            return m_enum_AuthError_AUTHORIZATION_PENDING.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::SLOW_DOWN:
            return m_enum_AuthError_SLOW_DOWN.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::INTERNAL_ERROR:
            return m_enum_AuthError_INTERNAL_ERROR.get();
        case aace::alexa::AuthProviderEngineInterface::AuthError::INVALID_CBL_CLIENT_ID:
            return m_enum_AuthError_INVALID_CBL_CLIENT_ID.get();
    }
}

jobject AlexaClientBinder::convert( aace::alexa::AlexaClient::ConnectionStatus status )
{
    switch( status )
    {
        case aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED:
            return m_enum_ConnectionStatus_DISCONNECTED.get();
        case aace::alexa::AlexaClient::ConnectionStatus::PENDING:
            return m_enum_ConnectionStatus_PENDING.get();
        case aace::alexa::AlexaClient::ConnectionStatus::CONNECTED:
            return m_enum_ConnectionStatus_CONNECTED.get();
    }
}

jobject AlexaClientBinder::convert( aace::alexa::AlexaClient::ConnectionChangedReason reason )
{
    switch( reason )
    {
        case aace::alexa::AlexaClient::ConnectionChangedReason::NONE:
            return m_enum_ConnectionChangedReason_NONE.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::SUCCESS:
            return m_enum_ConnectionChangedReason_SUCCESS.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::UNRECOVERABLE_ERROR:
            return m_enum_ConnectionChangedReason_UNRECOVERABLE_ERROR.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST:
            return m_enum_ConnectionChangedReason_ACL_CLIENT_REQUEST.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::ACL_DISABLED:
            return m_enum_ConnectionChangedReason_ACL_DISABLED.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::DNS_TIMEDOUT:
            return m_enum_ConnectionChangedReason_DNS_TIMEDOUT.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::CONNECTION_TIMEDOUT:
            return m_enum_ConnectionChangedReason_CONNECTION_TIMEDOUT.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::CONNECTION_THROTTLED:
            return m_enum_ConnectionChangedReason_CONNECTION_THROTTLED.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::INVALID_AUTH:
            return m_enum_ConnectionChangedReason_INVALID_AUTH.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::PING_TIMEDOUT:
            return m_enum_ConnectionChangedReason_PING_TIMEDOUT.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::WRITE_TIMEDOUT:
            return m_enum_ConnectionChangedReason_WRITE_TIMEDOUT.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::READ_TIMEDOUT:
            return m_enum_ConnectionChangedReason_READ_TIMEDOUT.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::FAILURE_PROTOCOL_ERROR:
            return m_enum_ConnectionChangedReason_FAILURE_PROTOCOL_ERROR.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::INTERNAL_ERROR:
            return m_enum_ConnectionChangedReason_INTERNAL_ERROR.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_INTERNAL_ERROR:
            return m_enum_ConnectionChangedReason_SERVER_INTERNAL_ERROR.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT:
            return m_enum_ConnectionChangedReason_SERVER_SIDE_DISCONNECT.get();
        case aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED:
            return m_enum_ConnectionChangedReason_SERVER_ENDPOINT_CHANGED.get();
    }
}

// JNI
#define ALEXACLIENT(cptr) ((AlexaClientBinder *) cptr)

extern "C" {

}

// END OF FILE
