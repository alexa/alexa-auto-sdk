/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_AUTHORIZATION_AUTHORIZATION_BINDER_H
#define AACE_JNI_AUTHORIZATION_AUTHORIZATION_BINDER_H

#include <AACE/Authorization/Authorization.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace authorization {

//
// AuthorizationHandler
//
class AuthorizationHandler : public aace::authorization::Authorization {
public:
    AuthorizationHandler(jobject obj);

    // aace::authorization::Authorization
    void eventReceived(const std::string& service, const std::string& request) override;
    void authorizationStateChanged(const std::string& service, AuthorizationState state) override;
    void authorizationError(const std::string& service, const std::string& error, const std::string& message) override;
    std::string getAuthorizationData(const std::string& service, const std::string& key) override;
    void setAuthorizationData(const std::string& service, const std::string& key, const std::string& data) override;

private:
    JObject m_obj;
};

//
// AuthorizationBinder
//
class AuthorizationBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AuthorizationBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_authorizationHandler;
    }

    std::shared_ptr<AuthorizationHandler> getAuthorization() {
        return m_authorizationHandler;
    }

private:
    std::shared_ptr<AuthorizationHandler> m_authorizationHandler;
};

//
// JAuthorizationState
//
class JAuthorizationStateConfig : public EnumConfiguration<AuthorizationHandler::AuthorizationState> {
public:
    using T = AuthorizationHandler::AuthorizationState;

    const char* getClassName() override {
        return "com/amazon/aace/authorization/Authorization$AuthorizationState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::AUTHORIZING, "AUTHORIZING"}, {T::AUTHORIZED, "AUTHORIZED"}, {T::UNAUTHORIZED, "UNAUTHORIZED"}};
    }
};

using JAuthorizationState = JEnum<AuthorizationHandler::AuthorizationState, JAuthorizationStateConfig>;

}  // namespace authorization
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_AUTHORIZATION_AUTHORIZATION_BINDER_H
