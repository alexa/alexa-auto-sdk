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

#ifndef AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_ENGINE_SERVICE_H
#define AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_ENGINE_SERVICE_H

#include <unordered_map>

#include <AACE/Engine/Core/EngineService.h>

#include "AuthorizationEngineImpl.h"
#include "AuthorizationServiceInterface.h"

namespace aace {
namespace engine {
namespace authorization {

class AuthorizationEngineService
        : public aace::engine::core::EngineService
        , public AuthorizationServiceInterface
        , public std::enable_shared_from_this<AuthorizationEngineService> {
public:
    DESCRIBE("aace.authorization", VERSION("1.0"))

private:
    AuthorizationEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~AuthorizationEngineService() = default;

    /// @name AuthorizationServiceInterface
    /// @{
    bool registerProvider(std::shared_ptr<AuthorizationProvider> provider, const std::string& service) override;
    std::shared_ptr<AuthorizationProvider> getProvider(const std::string& service) override;
    /// @}

protected:
    bool shutdown() override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::authorization::Authorization> authorization);

private:
    /// The reference to the authorization engine implementation
    std::shared_ptr<AuthorizationEngineImpl> m_authorizationEngineImpl;

    /// Holds the service and its associated @c AuthorizationProvider reference.
    std::unordered_map<std::string, std::weak_ptr<AuthorizationProvider>> m_authorizationProviderMap;

    /// To serialize the access to @c m_serviceAuthorizationProviderMap
    std::mutex m_mutex;
};

}  // namespace authorization
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUTHORIZATION_AUTHORIZATION_ENGINE_SERVICE_H
