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

#ifndef SAMPLEAPP_AUTHORIZATION_AUTH_PROVIDER_AUTHORIZATION_HANDLER_H
#define SAMPLEAPP_AUTHORIZATION_AUTH_PROVIDER_AUTHORIZATION_HANDLER_H

#include <chrono>

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include "AuthProviderAuthorizationListenerInterface.h"

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {

class ApplicationContext;  // forward declare
namespace authorization {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AuthProviderAuthorizationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Provides the implementation to handle the Auth Provider Authorization.
 */
class AuthProviderAuthorizationHandler {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};
    std::weak_ptr<AuthProviderAuthorizationListenerInterface> m_listener{};

protected:
    AuthProviderAuthorizationHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::weak_ptr<AuthProviderAuthorizationListenerInterface> listener);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AuthProviderAuthorizationHandler> {
        return std::shared_ptr<AuthProviderAuthorizationHandler>(new AuthProviderAuthorizationHandler(args...));
    }

    ~AuthProviderAuthorizationHandler();

    /** 
     * To check if the Auth Provider is supported.
     * 
     * @return Returns @c true if Auth Provider is supported, otherwise @c false.  
     */
    auto isAuthProviderSupported() -> bool;

    /** 
     * Start the Auth Provider authorization.
     */
    auto startAuthorization() -> void;

    /** 
     * Logout the Auth Provider authorization.
     */
    auto logout() -> void;

    /** 
     * Retrives the access token.
     * 
     * @return If available returns the token, otherwise empty string. 
     */
    auto getAccessToken() -> std::string;

    /** 
     * Saves the device info used for the Auth Provider authorization process.
     * 
     */
    auto saveDeviceInfo(const std::vector<json>& jsons) -> void;

private:
    /**
     * Show message to console. 
     */
    auto showMessage(const std::string& message, bool asError = false) -> void;

    /**
     * Reset state global variables.
     */
    auto resetGlobals() -> void;

    /**
     * Log entry.
     */
    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;

    /// Starts the access token refreshing thread.
    void startRefreshThread();

    /// Stops the refreshing thread.
    void stopRefreshThread();

    /// Thread that handles the refreshing of the access token.
    void handleRefreshingToken();

    /// Returns if the refreshing token thread needs to be stopped.
    bool isStopping();

private:
    /// Application context
    std::shared_ptr<ApplicationContext> m_applicationContext{};

    /// Client Id
    std::string m_clientId;

    /// Device serial number
    std::string m_deviceSerialNumber;

    /// Product id
    std::string m_productId;

    /// Console
    std::weak_ptr<View> m_console{};

    /// The listener interface for the handler event.
    std::weak_ptr<AuthProviderAuthorizationListenerInterface> listener;

    /// Used to wake refreshing thread.
    std::condition_variable m_wake;

    /// Flag to indicate if the refreshing thread needs to be stopped.
    bool m_isStopping{false};

    /// The reference to the refreshing thread.
    std::thread m_refreshTokenThread;

    /// The access token provided by LWA.
    std::string m_accessToken;

    /// The refresh token provided by LWA.
    std::string m_refreshToken;

    /// The time at which access token needs to be refreshed.
    std::chrono::seconds m_tokenExpirationTime;

    /// The path to consumer token script.
    std::string m_consumerTokenScript;

    /// The path to device registration script.
    std::string m_deviceRegistrationScript;

    /// The path to refresh token script.
    std::string m_refreshTokenScript;

    /// The path to device deregistration script.
    std::string m_deviceDeregistrationScript;

    /// Indicates if the thread is active.
    std::atomic<bool> m_threadActive{false};

    /// To serialize the access to this module API.
    std::mutex m_mutex;

    /// Serializes the access to the refresh token.
    std::mutex m_refreshTokenMutex;

    /// Thread for responding to authorization request.
    Executor m_executer;
};

}  // namespace authorization
}  // namespace sampleApp

#endif  // SAMPLEAPP_AUTHORIZATION_AUTH_PROVIDER_AUTHORIZATION_HANDLER_H
