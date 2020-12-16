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

#ifndef AACE_ENGINE_ALEXA_AUTH_PROVIDER_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_AUTH_PROVIDER_ENGINE_IMPL_H

#include <AVSCommon/Utils/RequiresShutdown.h>

#include <AACE/Alexa/AuthProvider.h>
#include <AACE/Engine/Authorization/AuthorizationProviderListenerInterface.h>

#include "AlexaAuthorizationProvider.h"

namespace aace {
namespace engine {
namespace alexa {

class AuthProviderEngineImpl
        : public aace::alexa::AuthProviderEngineInterface
        , public authorization::AuthorizationProviderListenerInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<AuthProviderEngineImpl> {
private:
    /**
     * Constructor
     * 
     * @param authProvider The reference to the platform implemenation of @c AuthProvider
     */
    AuthProviderEngineImpl(std::shared_ptr<aace::alexa::AuthProvider> authProvider);

    /**
     * Initializes the object.
     * 
     * @param authorizationManagerInterface To carry out the required authorization flow with @c AuthorizationManager.
     * @param customerDataManager The @c CustomerDataManager object that will track the CustomerDataHandler.
     */
    bool initialize(std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface);

public:
    static std::shared_ptr<AuthProviderEngineImpl> create(
        std::shared_ptr<aace::alexa::AuthProvider> authProvider,
        std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface);

    /**
     * Start the Auth Provider Authorization.
     */
    void startAuthorization();

    /**
     * Stop the Auth Provider Authorization.
     */
    void stopAuthorization();

    // AuthProviderEngineInterface
    void onAuthStateChanged(AuthState authState, AuthError authError) override;

    /// @name AuthorizationProviderListenerInterface
    /// @{
    void onAuthorizationStateChanged(
        const std::string& service,
        AuthorizationProviderListenerInterface::AuthorizationState state) override;
    void onAuthorizationError(const std::string& service, const std::string& error, const std::string& message)
        override;
    void onEventReceived(const std::string& service, const std::string& request) override;
    std::string onGetAuthorizationData(const std::string& service, const std::string& key) override;
    void onSetAuthorizationData(const std::string& service, const std::string& key, const std::string& data) override;
    /// @}

protected:
    virtual void doShutdown() override;

private:
    /// Represents the possible states of AuthProviderEngineImpl
    enum class AuthProviderEngineState {
        /// Initial State
        INITIALIZED,

        /// Same as AuthorizationState::AUTHORIZING
        AUTHORIZING,

        /// Same as AuthorizationState::AUTHORIZED
        AUTHORIZED,

        /// Same as AuthorizationState::UNAUTHORIZED
        UNAUTHORIZED,

        /// Represents the error when onAuthorizationError() occurs
        ERROR
    };

    /// Represents the current state of AuthProviderEngineImpl
    AuthProviderEngineState m_state;

    /// Client interface that provides auth token.
    std::shared_ptr<aace::alexa::AuthProvider> m_authProviderPlatformInterface;

    /// Carries out the AuthProvider authorization.
    std::shared_ptr<AlexaAuthorizationProvider> m_alexaAuthorizationProvider;

    /// Synchronizes @c AuthProviderEngineInterface calls with @c AuthorizationProviderListenerInterface callbacks that update @c m_state.
    std::condition_variable m_cv;

    /// Indicates falling back to AUTHORIZING state after log out.
    bool m_resetToAuthorizingState;

    /// To serialize access to @c m_state
    std::mutex m_mutex;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_AUTH_PROVIDER_ENGINE_IMPL_H
