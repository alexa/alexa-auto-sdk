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

#ifndef AACE_ENGINE_ALEXA_ALEXA_AUTHORIZATION_PROVIDER_H
#define AACE_ENGINE_ALEXA_ALEXA_AUTHORIZATION_PROVIDER_H

#include <unordered_set>
#include <mutex>
#include <memory>

#include <AACE/Engine/Authorization/AuthorizationProvider.h>
#include <AACE/Engine/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include "AuthorizationAdapterInterface.h"
#include "AuthorizationManagerInterface.h"

namespace aace {
namespace engine {
namespace alexa {

class AlexaAuthorizationProvider
        : public aace::engine::authorization::AuthorizationProvider
        , public AuthorizationAdapterInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<AlexaAuthorizationProvider> {
private:
    /**
     * Constructor
     * 
     * @param service The name to be used for registering with the @c AuthorizationManager.
     */
    AlexaAuthorizationProvider(const std::string& service);

    /**
     * Initializes the object.
     * 
     * @param authorizationManagerInterface To carry out the required authorization flow with @c AuthorizationManager.
     */
    bool initialize(std::shared_ptr<AuthorizationManagerInterface> authorizationManagerInterface);

public:
    /**
     * Creates the reference of @c AlexaAuthorizationProvider
     * 
     * @param service The name to be used for registering with the @c AuthorizationManager.
     * @param authorizationManagerInterface To carry out the required authorization flow with @c AuthorizationManager.
     * @return On successfull returns the valid reference otherwise @c nullptr.
     */
    static std::shared_ptr<AlexaAuthorizationProvider> create(
        const std::string& service,
        std::shared_ptr<AuthorizationManagerInterface> authorizationManagerInterface);

    /// @name AuthorizationProvider
    /// @{
    bool startAuthorization(const std::string& data) override;
    bool cancelAuthorization() override;
    bool logout() override;
    bool sendEvent(const std::string& data) override;
    /// @}

    /// @name AuthorizationAdapterInterface
    /// @{
    void deregister() override;
    std::string getAuthToken() override;
    void onAuthFailure(const std::string& token) override;
    /// @}

protected:
    /// @name RequiresShutdown
    /// @{
    void doShutdown() override;
    /// @}

private:
    //Alias for readability
    using AuthorizationState = authorization::AuthorizationProviderListenerInterface::AuthorizationState;

    /// Holds the service name to be used for @c AuthorizationManager and @c AuthorizationProviderListenerInterface
    std::string m_service;

    /// Represents the current authorization state.
    AuthorizationState m_currentAuthState;

    /// Reference to access the authorization manager interface.
    std::weak_ptr<AuthorizationManagerInterface> m_authorizationManager;

    /// To serialize the access to this modules API.
    std::mutex m_mutex;

    /// This is the worker thread for the @c AlexaAuthorizationProvider.
    aace::engine::utils::threading::Executor m_executor;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_ALEXA_AUTHORIZATION_PROVIDER_H
