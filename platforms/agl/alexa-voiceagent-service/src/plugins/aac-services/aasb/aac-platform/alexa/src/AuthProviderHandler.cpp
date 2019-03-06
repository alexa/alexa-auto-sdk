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
#include "AuthProviderHandler.h"

#include <aasb/Consts.h>
#include "PlatformSpecificLoggingMacros.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace alexa {

using namespace aasb::bridge;

const std::string TAG = "aasb::alexa::AuthProviderHandler";

std::shared_ptr<AuthProviderHandler> AuthProviderHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger) {
    auto authProviderHandler = std::shared_ptr<AuthProviderHandler>(new AuthProviderHandler());

    authProviderHandler->m_logger = logger;

    return authProviderHandler;
}

AuthProviderHandler::AuthProviderHandler() {
    m_authToken = "";
    m_authState = AuthState::UNINITIALIZED;
}

std::string AuthProviderHandler::getAuthToken() {
    if (m_authToken.empty())
        AASB_INFO("Auth token not set.");
    else
        AASB_INFO("Auth token %s:", m_authToken.c_str());
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return m_authToken;
}

AuthState AuthProviderHandler::getAuthState() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return m_authState;
}

void AuthProviderHandler::clearAuthToken() {
    m_authToken = "";
    m_authState = AuthState::UNINITIALIZED;
    m_logger->log(Level::INFO, TAG, "auth toke cleared");
    authStateChanged(AuthState::UNINITIALIZED, AuthError::NO_ERROR);
}

void AuthProviderHandler::onReceivedEvent(const std::string& action, const std::string& payload) {
    if (action == ACTION_CLEAR_AUTH_TOKEN) {
        clearAuthToken();
        return;
    }

    if (action == ACTION_SET_AUTH_TOKEN) {
        setAuthToken(payload);
        return;
    }

    AASB_ERROR("AuthProviderHandler: action %s is unknown.", action.c_str());
}

void AuthProviderHandler::setAuthToken(const std::string& token) {
    m_authToken = token;
    // Let the engine know that the auth state is refreshed.
    if (!m_authToken.empty()) {
        m_authState = AuthState::REFRESHED;
        m_logger->log(Level::INFO, TAG, "auth token saved");
        authStateChanged(AuthState::REFRESHED, AuthError::NO_ERROR);
    }
}

}  // namespace alexa
}  // namespace aasb