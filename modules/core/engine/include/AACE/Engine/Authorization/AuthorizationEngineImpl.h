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

#ifndef AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_ENGINE_IMPL_H
#define AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_ENGINE_IMPL_H

#include <memory>

#include <AACE/Authorization/Authorization.h>
#include <AACE/Authorization/AuthorizationEngineInterface.h>

#include "AuthorizationProviderListenerInterface.h"
#include "AuthorizationServiceInterface.h"

namespace aace {
namespace engine {
namespace authorization {

class AuthorizationEngineImpl
        : public aace::authorization::AuthorizationEngineInterface
        , public AuthorizationProviderListenerInterface {
private:
    AuthorizationEngineImpl(
        std::shared_ptr<aace::authorization::Authorization> authorizationPlatformInterface,
        std::shared_ptr<AuthorizationServiceInterface> authorizationService);

public:
    static std::shared_ptr<AuthorizationEngineImpl> create(
        std::shared_ptr<aace::authorization::Authorization> authorizationPlatformInterface,
        std::shared_ptr<AuthorizationServiceInterface> authorizationService);

    /// @name AuthorizationEngineInterface
    /// @{
    void onStartAuthorization(const std::string& service, const std::string& data) override;
    void onCancelAuthorization(const std::string& service) override;
    void onSendEvent(const std::string& service, const std::string& event) override;
    void onLogout(const std::string& service) override;
    /// @}

    /// @name AuthorizationProviderListenerInterface
    /// @{
    void onAuthorizationStateChanged(
        const std::string& service,
        AuthorizationProviderListenerInterface::AuthorizationState state) override;
    void onAuthorizationError(const std::string& service, const std::string& error, const std::string& message)
        override;
    void onEventReceived(const std::string& service, const std::string& event) override;
    std::string onGetAuthorizationData(const std::string& service, const std::string& key) override;
    void onSetAuthorizationData(const std::string& service, const std::string& key, const std::string& data) override;
    /// @}
    void doShutDown();

private:
    /// Authorization platform interface handler reference
    std::shared_ptr<aace::authorization::Authorization> m_authorizationPlatformInterface;

    /// Reference to get the registered authorization service
    std::shared_ptr<AuthorizationServiceInterface> m_authorizationService;
};

}  // namespace authorization
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_ENGINE_IMPL_H
