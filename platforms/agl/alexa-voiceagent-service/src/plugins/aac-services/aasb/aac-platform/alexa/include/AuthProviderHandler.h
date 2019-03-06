/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AASB_ALEXA_AUTHPROVIDERHANDLER_H
#define AASB_ALEXA_AUTHPROVIDERHANDLER_H

#include <memory>

#include <AACE/Alexa/AuthProvider.h>

#include "LoggerHandler.h"

/**
 * Describes the state of client authorization with AVS
 * @sa @c aace::alexa::AuthProviderEngineInterface::AuthState
 */
using AuthState = aace::alexa::AuthProviderEngineInterface::AuthState;

namespace aasb {
namespace alexa {

class AuthProviderHandler : public aace::alexa::AuthProvider {
public:
    ~AuthProviderHandler() {
    }
    static std::shared_ptr<AuthProviderHandler> create(std::shared_ptr<aasb::core::logger::LoggerHandler> logger);

    /// @name aace::alexa::AuthProvider Functions
    /// @{
    std::string getAuthToken() override;
    AuthState getAuthState() override;
    /// @}

    void onReceivedEvent(const std::string& action, const std::string& payload);

private:
    AuthProviderHandler();

    void setAuthToken(const std::string& authToken);
    void clearAuthToken();

    std::string m_authToken;
    AuthState m_authState;
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_AUTHPROVIDERHANDLER_H