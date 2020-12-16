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

#ifndef SAMPLEAPP_AUTHORIZATION_AUTHORIZATIONHANDLER_H
#define SAMPLEAPP_AUTHORIZATION_AUTHORIZATIONHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Authorization/Authorization.h>

#include "AuthProviderAuthorizationHandler.h"
#include "AuthProviderAuthorizationListenerInterface.h"

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {

class ApplicationContext;  // forward declare
namespace authorization {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AuthorizationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Provides a sample implementation of the @c Authorization platform interface. 
 */
class AuthorizationHandler
        : public aace::authorization::Authorization
        , public AuthProviderAuthorizationListenerInterface
        , public std::enable_shared_from_this<AuthorizationHandler> {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    AuthorizationHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AuthorizationHandler> {
        auto var = std::shared_ptr<AuthorizationHandler>(new AuthorizationHandler(args...));
        var->initialize();
        return var;
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;
    auto saveDeviceInfo(const std::vector<json>& jsons) -> void;

    // aace::authorization::Authorization interface
    auto eventReceived(const std::string& service, const std::string& event) -> void override;
    auto authorizationStateChanged(const std::string& service, AuthorizationState state) -> void override;
    auto authorizationError(const std::string& service, const std::string& error, const std::string& message)
        -> void override;
    auto getAuthorizationData(const std::string& service, const std::string& key) -> std::string override;
    auto setAuthorizationData(const std::string& service, const std::string& key, const std::string& data)
        -> void override;

    // AuthProviderAuthorizationListenerInterface
    void sendHandlerEvent(const std::string& service, const std::string& data) override;

private:
    /**
     * Initializes the object. 
     */
    auto initialize() -> void;

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

    /**
     * Setup all the event listeners.
     */
    auto setupUI() -> void;

private:
    /// Application context
    std::shared_ptr<ApplicationContext> m_applicationContext{};

    /// Console
    std::weak_ptr<View> m_console{};

    /// Chosen authorization
    std::string m_selectedAuthorization;

    /// Provides the implemenation of Auth Provider Authorization
    std::shared_ptr<AuthProviderAuthorizationHandler> m_authProviderHandler{};

    /// To wait on state change notification
    std::condition_variable m_cv;

    /// Mutex for the @c m_cv
    std::mutex m_mutex;
};

}  // namespace authorization
}  // namespace sampleApp

#endif  // SAMPLEAPP_AUTHORIZATION_AUTHORIZATION_H
