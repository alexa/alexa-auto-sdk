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

// aace/alexa/AuthProviderBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/AuthProviderBinder.h"

void AuthProviderBinder::initialize( JNIEnv* env )
{
    m_javaMethod_getAuthToken = env->GetMethodID( getJavaClass(), "getAuthToken", "()Ljava/lang/String;" );
    m_javaMethod_getAuthState = env->GetMethodID( getJavaClass(), "getAuthState", "()Lcom/amazon/aace/alexa/AuthProvider$AuthState;" );

    // AuthState
    jclass authStateEnumClass = env->FindClass( "com/amazon/aace/alexa/AuthProvider$AuthState" );
    m_enum_AuthState_UNINITIALIZED = NativeLib::FindEnum( env, authStateEnumClass, "UNINITIALIZED", "Lcom/amazon/aace/alexa/AuthProvider$AuthState;" );
    m_enum_AuthState_REFRESHED = NativeLib::FindEnum( env, authStateEnumClass, "REFRESHED", "Lcom/amazon/aace/alexa/AuthProvider$AuthState;" );
    m_enum_AuthState_EXPIRED = NativeLib::FindEnum( env, authStateEnumClass, "EXPIRED", "Lcom/amazon/aace/alexa/AuthProvider$AuthState;" );
    m_enum_AuthState_UNRECOVERABLE_ERROR = NativeLib::FindEnum( env, authStateEnumClass, "UNRECOVERABLE_ERROR", "Lcom/amazon/aace/alexa/AuthProvider$AuthState;" );

    // AuthError
    jclass authErrorEnumClass = env->FindClass( "com/amazon/aace/alexa/AuthProvider$AuthError" );
    m_enum_AuthError_NO_ERROR = NativeLib::FindEnum( env, authErrorEnumClass, "NO_ERROR", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_UNKNOWN_ERROR = NativeLib::FindEnum( env, authErrorEnumClass, "UNKNOWN_ERROR", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_AUTHORIZATION_FAILED = NativeLib::FindEnum( env, authErrorEnumClass, "AUTHORIZATION_FAILED", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_UNAUTHORIZED_CLIENT = NativeLib::FindEnum( env, authErrorEnumClass, "UNAUTHORIZED_CLIENT", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_SERVER_ERROR = NativeLib::FindEnum( env, authErrorEnumClass, "SERVER_ERROR", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_INVALID_REQUEST = NativeLib::FindEnum( env, authErrorEnumClass, "INVALID_REQUEST", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_INVALID_VALUE = NativeLib::FindEnum( env, authErrorEnumClass, "INVALID_VALUE", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_AUTHORIZATION_EXPIRED = NativeLib::FindEnum( env, authErrorEnumClass, "AUTHORIZATION_EXPIRED", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_UNSUPPORTED_GRANT_TYPE = NativeLib::FindEnum( env, authErrorEnumClass, "UNSUPPORTED_GRANT_TYPE", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_INVALID_CODE_PAIR = NativeLib::FindEnum( env, authErrorEnumClass, "INVALID_CODE_PAIR", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_AUTHORIZATION_PENDING = NativeLib::FindEnum( env, authErrorEnumClass, "AUTHORIZATION_PENDING", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_SLOW_DOWN = NativeLib::FindEnum( env, authErrorEnumClass, "SLOW_DOWN", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_INTERNAL_ERROR = NativeLib::FindEnum( env, authErrorEnumClass, "INTERNAL_ERROR", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );
    m_enum_AuthError_INVALID_CBL_CLIENT_ID = NativeLib::FindEnum( env, authErrorEnumClass, "INVALID_CBL_CLIENT_ID", "Lcom/amazon/aace/alexa/AuthProvider$AuthError;" );

}

std::string AuthProviderBinder::getAuthToken()
{
    std::string result = "";

    if( getJavaObject() != nullptr && m_javaMethod_getAuthToken != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring jstr = (jstring) context.getEnv()->CallObjectMethod( getJavaObject(), m_javaMethod_getAuthToken );

            if( jstr != nullptr ) {
                result = NativeLib::convert( context.getEnv(), jstr );
            }
        }
    }

    return result;
}

aace::alexa::AuthProvider::AuthState AuthProviderBinder::getAuthState()
{
    aace::alexa::AuthProvider::AuthState result = aace::alexa::AuthProvider::AuthState::UNRECOVERABLE_ERROR;

    if( getJavaObject() != nullptr && m_javaMethod_getAuthState != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = convertAuthState( context.getEnv(), context.getEnv()->CallObjectMethod( getJavaObject(), m_javaMethod_getAuthState ));
        }
    }
    return result;
}

jobject AuthProviderBinder::convert( aace::alexa::AuthProviderEngineInterface::AuthState state )
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

aace::alexa::AuthProviderEngineInterface::AuthState AuthProviderBinder::convertAuthState( JNIEnv* env, jobject obj )
{
    if( m_enum_AuthState_UNINITIALIZED.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthState::UNINITIALIZED;
    }
    else if( m_enum_AuthState_REFRESHED.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthState::REFRESHED;
    }
    else if( m_enum_AuthState_EXPIRED.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthState::EXPIRED;
    }
    else if( m_enum_AuthState_UNRECOVERABLE_ERROR.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthState::UNRECOVERABLE_ERROR;
    }
    else {
        return aace::alexa::AuthProviderEngineInterface::AuthState::UNRECOVERABLE_ERROR; // anything for undefined?
    }
}

jobject AuthProviderBinder::convert( aace::alexa::AuthProviderEngineInterface::AuthError error )
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

aace::alexa::AuthProviderEngineInterface::AuthError AuthProviderBinder::convertAuthError( JNIEnv* env, jobject obj )
{
    if( m_enum_AuthError_NO_ERROR.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthError::NO_ERROR;
    }
    else if( m_enum_AuthError_UNKNOWN_ERROR.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthError::UNKNOWN_ERROR;
    }
    else if( m_enum_AuthError_AUTHORIZATION_FAILED.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthError::AUTHORIZATION_FAILED;
    }
    else if( m_enum_AuthError_UNAUTHORIZED_CLIENT.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthError::UNAUTHORIZED_CLIENT;
    }
    else if( m_enum_AuthError_SERVER_ERROR.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthError::SERVER_ERROR;
    }
    else if( m_enum_AuthError_INVALID_REQUEST.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthError::INVALID_REQUEST;
    }
    else if( m_enum_AuthError_AUTHORIZATION_EXPIRED.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthError::AUTHORIZATION_EXPIRED;
    }
    else if( m_enum_AuthError_UNSUPPORTED_GRANT_TYPE.isSameObject( env, obj ) ) {
        return aace::alexa::AuthProviderEngineInterface::AuthError::UNSUPPORTED_GRANT_TYPE;
    }
    else {
        return aace::alexa::AuthProviderEngineInterface::AuthError::UNSUPPORTED_GRANT_TYPE;
    }
}

// JNI
#define AUTHPROVIDER(cptr) ((AuthProviderBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_AuthProvider_authStateChange( JNIEnv * env , jobject /* this */, jlong cptr, jobject authState, jobject authError ) {
    AUTHPROVIDER(cptr)->authStateChanged( AUTHPROVIDER(cptr)->convertAuthState( env, authState ), AUTHPROVIDER(cptr)->convertAuthError( env, authError ));
}

}

// END OF FILE