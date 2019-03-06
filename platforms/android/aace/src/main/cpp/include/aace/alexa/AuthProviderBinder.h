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

// aace/alexa/AuthProviderBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_AUTH_PROVIDER_BINDER_H
#define AACE_ALEXA_AUTH_PROVIDER_BINDER_H

#include "AACE/Alexa/AuthProvider.h"
#include "aace/core/PlatformInterfaceBinder.h"

class AuthProviderBinder : public PlatformInterfaceBinder, public aace::alexa::AuthProvider {
public:
    AuthProviderBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    std::string getAuthToken() override;
    aace::alexa::AuthProvider::AuthState getAuthState() override;

public:
    aace::alexa::AuthProviderEngineInterface::AuthState convertAuthState( JNIEnv* env, jobject obj );
    aace::alexa::AuthProviderEngineInterface::AuthError convertAuthError( JNIEnv* env, jobject obj );

private:
    jobject convert( aace::alexa::AuthProviderEngineInterface::AuthState state );
    jobject convert( aace::alexa::AuthProviderEngineInterface::AuthError error );

private:
    jmethodID m_javaMethod_getAuthToken = nullptr;
    jmethodID m_javaMethod_getAuthState = nullptr;

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
};

#endif //AACE_ALEXA_AUTH_PROVIDER_BINDER_H
