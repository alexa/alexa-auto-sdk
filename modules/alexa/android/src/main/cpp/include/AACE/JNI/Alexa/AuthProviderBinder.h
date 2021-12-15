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

#ifndef AACE_JNI_ALEXA_AUTH_PROVIDER_BINDER_H
#define AACE_JNI_ALEXA_AUTH_PROVIDER_BINDER_H

#include <AACE/Alexa/AuthProvider.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class AuthProviderHandler : public aace::alexa::AuthProvider {
public:
    AuthProviderHandler(jobject obj);

    // aace::alexa::AuthProvider
    std::string getAuthToken() override;
    AuthState getAuthState() override;
    void authFailure(const std::string& token) override;

private:
    JObject m_obj;
};

class AuthProviderBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AuthProviderBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_authProviderHandler;
    }

    std::shared_ptr<AuthProviderHandler> getAuthProvider() {
        return m_authProviderHandler;
    }

private:
    std::shared_ptr<AuthProviderHandler> m_authProviderHandler;
};

//
// JAuthState
//

class JAuthStateConfig : public EnumConfiguration<AuthProviderHandler::AuthState> {
public:
    using T = AuthProviderHandler::AuthState;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/AuthProvider$AuthState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {
            {T::UNINITIALIZED, "UNINITIALIZED"},
            {T::REFRESHED, "REFRESHED"},
            {T::EXPIRED, "EXPIRED"},
            {T::UNRECOVERABLE_ERROR, "UNRECOVERABLE_ERROR"},
        };
    }
};

using JAuthState = JEnum<AuthProviderHandler::AuthState, JAuthStateConfig>;

//
// JAuthError
//

class JAuthErrorConfig : public EnumConfiguration<AuthProviderHandler::AuthError> {
public:
    using T = AuthProviderHandler::AuthError;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/AuthProvider$AuthError";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::NO_ERROR, "NO_ERROR"},
                {T::UNKNOWN_ERROR, "UNKNOWN_ERROR"},
                {T::AUTHORIZATION_FAILED, "AUTHORIZATION_FAILED"},
                {T::UNAUTHORIZED_CLIENT, "UNAUTHORIZED_CLIENT"},
                {T::SERVER_ERROR, "SERVER_ERROR"},
                {T::INVALID_REQUEST, "INVALID_REQUEST"},
                {T::INVALID_VALUE, "INVALID_VALUE"},
                {T::AUTHORIZATION_EXPIRED, "AUTHORIZATION_EXPIRED"},
                {T::UNSUPPORTED_GRANT_TYPE, "UNSUPPORTED_GRANT_TYPE"},
                {T::INVALID_CODE_PAIR, "INVALID_CODE_PAIR"},
                {T::AUTHORIZATION_PENDING, "AUTHORIZATION_PENDING"},
                {T::SLOW_DOWN, "SLOW_DOWN"},
                {T::INTERNAL_ERROR, "INTERNAL_ERROR"},
                {T::INVALID_CBL_CLIENT_ID, "INVALID_CBL_CLIENT_ID"}};
    }
};

using JAuthError = JEnum<AuthProviderHandler::AuthError, JAuthErrorConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_AUTH_PROVIDER_BINDER_H