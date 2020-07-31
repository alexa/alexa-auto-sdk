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

#ifndef AACE_JNI_ALEXA_ALEXA_CLIENT_BINDER_H
#define AACE_JNI_ALEXA_ALEXA_CLIENT_BINDER_H

#include <AACE/Alexa/AlexaClient.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

//
// AlexaClientHandler
//

class AlexaClientHandler : public aace::alexa::AlexaClient {
public:
    AlexaClientHandler(jobject obj);

    // aace::alexa::AlexaClient
    void dialogStateChanged(DialogState state) override;
    void authStateChanged(AuthState state, AuthError error) override;
    void connectionStatusChanged(ConnectionStatus status, ConnectionChangedReason reason) override;

private:
    JObject m_obj;
};

//
// AlexaClientBinder
//

class AlexaClientBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AlexaClientBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_alexaClientHandler;
    }

    std::shared_ptr<AlexaClientHandler> getAlexaClient() {
        return m_alexaClientHandler;
    }

private:
    std::shared_ptr<AlexaClientHandler> m_alexaClientHandler;
};

//
// JDialogState
//

class JAlexaClientDialogStateConfig : public EnumConfiguration<AlexaClientHandler::DialogState> {
public:
    using T = AlexaClientHandler::DialogState;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/AlexaClient$DialogState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::IDLE, "IDLE"},
                {T::LISTENING, "LISTENING"},
                {T::EXPECTING, "EXPECTING"},
                {T::THINKING, "THINKING"},
                {T::SPEAKING, "SPEAKING"}};
    }
};

using JDialogState = JEnum<AlexaClientHandler::DialogState, JAlexaClientDialogStateConfig>;

//
// JAuthState
//

class JAlexaClientAuthStateConfig : public EnumConfiguration<AlexaClientHandler::AuthState> {
public:
    using T = AlexaClientHandler::AuthState;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/AlexaClient$AuthState";
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

using JAuthState = JEnum<AlexaClientHandler::AuthState, JAlexaClientAuthStateConfig>;

//
// JAuthError
//

class JAlexaClientAuthErrorConfig : public EnumConfiguration<AlexaClientHandler::AuthError> {
public:
    using T = AlexaClientHandler::AuthError;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/AlexaClient$AuthError";
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

using JAuthError = JEnum<AlexaClientHandler::AuthError, JAlexaClientAuthErrorConfig>;

//
// JConnectionStatus
//

class JAlexaClientConnectionStatusConfig : public EnumConfiguration<AlexaClientHandler::ConnectionStatus> {
public:
    using T = AlexaClientHandler::ConnectionStatus;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/AlexaClient$ConnectionStatus";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::DISCONNECTED, "DISCONNECTED"}, {T::PENDING, "PENDING"}, {T::CONNECTED, "CONNECTED"}};
    }
};

using JConnectionStatus = JEnum<AlexaClientHandler::ConnectionStatus, JAlexaClientConnectionStatusConfig>;

//
// JConnectionChangedReason
//

class JAlexaClientConnectionChangedReasonConfig
        : public EnumConfiguration<AlexaClientHandler::ConnectionChangedReason> {
public:
    using T = AlexaClientHandler::ConnectionChangedReason;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/AlexaClient$ConnectionChangedReason";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::NONE, "NONE"},
                {T::SUCCESS, "SUCCESS"},
                {T::UNRECOVERABLE_ERROR, "UNRECOVERABLE_ERROR"},
                {T::ACL_CLIENT_REQUEST, "ACL_CLIENT_REQUEST"},
                {T::ACL_DISABLED, "ACL_DISABLED"},
                {T::DNS_TIMEDOUT, "DNS_TIMEDOUT"},
                {T::CONNECTION_TIMEDOUT, "CONNECTION_TIMEDOUT"},
                {T::CONNECTION_THROTTLED, "CONNECTION_THROTTLED"},
                {T::INVALID_AUTH, "INVALID_AUTH"},
                {T::PING_TIMEDOUT, "PING_TIMEDOUT"},
                {T::WRITE_TIMEDOUT, "WRITE_TIMEDOUT"},
                {T::READ_TIMEDOUT, "READ_TIMEDOUT"},
                {T::FAILURE_PROTOCOL_ERROR, "FAILURE_PROTOCOL_ERROR"},
                {T::INTERNAL_ERROR, "INTERNAL_ERROR"},
                {T::SERVER_INTERNAL_ERROR, "SERVER_INTERNAL_ERROR"},
                {T::SERVER_SIDE_DISCONNECT, "SERVER_SIDE_DISCONNECT"},
                {T::SERVER_ENDPOINT_CHANGED, "SERVER_ENDPOINT_CHANGED"}};
    }
};

using JConnectionChangedReason =
    JEnum<AlexaClientHandler::ConnectionChangedReason, JAlexaClientConnectionChangedReasonConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_ALEXA_CLIENT_BINDER_H
