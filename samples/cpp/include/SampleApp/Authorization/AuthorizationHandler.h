/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AuthProviderAuthorizationHandler.h"
#include "AuthProviderAuthorizationListenerInterface.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Authorization/Authorization/AuthorizationState.h>

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

class AuthorizationHandler
        : public AuthProviderAuthorizationListenerInterface
        , public std::enable_shared_from_this<AuthorizationHandler> {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    AuthorizationHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

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

    // Function to start the authorization if we were successfully authorized before
    auto startAuth() -> void;

private:
    auto subscribeToAASBMessages() -> void;

    /**
     * Handles the EventReceived message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleEventReceivedMessage(const std::string& message);

    /**
     * Handles the AuthorizationStateChanged message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAuthorizationStateChangedMessage(const std::string& message);

    /**
     * Handles the AuthorizationError message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAuthorizationErrorMessage(const std::string& message);

    /**
     * Handles the GetAuthorizationData message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetAuthorizationDataMessage(const std::string& message);

    /**
     * Handles the SetAuthorizationData message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSetAuthorizationDataMessage(const std::string& message);

    /**
     * Provides the implementation for a received authorization event.
     */
    void eventReceived(const std::string& service, const std::string& event);

    /**
     * Provides the implementation for when the authorization state changed.
     */
    void authorizationStateChanged(
        const std::string& service,
        aasb::message::authorization::authorization::AuthorizationState state);

    /**
     * Provides the implementation for when an authorization error happens.
     */
    void authorizationError(const std::string& service, const std::string& error, const std::string& message);

    /**
     * Provides the implementation for getting authorization data.
     */
    std::string getAuthorizationData(const std::string& service, const std::string& key);

    /**
     * Provides the implementation for setting authorization data.
     */
    void setAuthorizationData(const std::string& service, const std::string& key, const std::string& data);

    // AuthProviderAuthorizationListenerInterface
    void sendHandlerEvent(const std::string& service, const std::string& data) override;

    /**
     * Notifies the Engine to start the authorization process.
     */
    void startAuthorization(const std::string& service, const std::string& data);

    /**
     * Notifies the Engine to cancel the authorization process.
     */
    void cancelAuthorization(const std::string& service);

    /**
     * Notifies the Engine of an authorization event.
     */
    void sendEvent(const std::string& service, const std::string& event);

    /**
     * Notifies the Engine to log out of an authorization.
     */
    void logout(const std::string& service);

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

    /// Provides the implementation of Auth Provider Authorization
    std::shared_ptr<AuthProviderAuthorizationHandler> m_authProviderHandler{};

    /// To wait on state change notification
    std::condition_variable m_cv;

    /// Mutex for the @c m_cv
    std::mutex m_mutex;
};

}  // namespace authorization
}  // namespace sampleApp

#endif  // SAMPLEAPP_AUTHORIZATION_AUTHORIZATION_H
