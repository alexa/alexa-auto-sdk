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

#include <typeinfo>

#include <AACE/Engine/Authorization/AuthorizationEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace authorization {

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.authorization.AuthorizationEngineService");

// register the service
REGISTER_SERVICE(AuthorizationEngineService)

AuthorizationEngineService::AuthorizationEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool AuthorizationEngineService::shutdown() {
    if (m_authorizationEngineImpl != nullptr) {
        m_authorizationEngineImpl->doShutDown();
        m_authorizationEngineImpl.reset();
    }
    m_authorizationProviderMap.clear();
    return true;
}

bool AuthorizationEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::authorization::Authorization>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool AuthorizationEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::authorization::Authorization> authorization) {
    try {
        ThrowIfNotNull(m_authorizationEngineImpl, "platformInterfaceAlreadyRegistered");
        m_authorizationEngineImpl = AuthorizationEngineImpl::create(authorization, shared_from_this());

        ThrowIfNot(
            registerServiceInterface<AuthorizationServiceInterface>(shared_from_this()),
            "registerAuthorizationServiceInterfaceFailed");

        for (const auto& it : m_authorizationProviderMap) {
            auto provider_lock = it.second.lock();
            ThrowIfNull(provider_lock, "invalidProviderReference");
            provider_lock->setListener(m_authorizationEngineImpl);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<Authorization>").d("reason", ex.what()));
        return false;
    }
}

bool AuthorizationEngineService::registerProvider(
    std::shared_ptr<AuthorizationProvider> provider,
    const std::string& service) {
    try {
        ThrowIfNull(provider, "invalidAuthorizationProvider");
        ThrowIf(service.empty(), "invalidService");

        ThrowIf(
            m_authorizationProviderMap.find(service) != m_authorizationProviderMap.end(),
            "authorizationProviderAlreadyRegisteredForService");
        m_authorizationProviderMap[service] = provider;

        if (m_authorizationEngineImpl) {
            provider->setListener(m_authorizationEngineImpl);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AuthorizationProvider> AuthorizationEngineService::getProvider(const std::string& service) {
    try {
        auto provider = m_authorizationProviderMap.find(service);
        ThrowIf(provider == m_authorizationProviderMap.end(), "authorizationProviderNotFound");

        auto provider_lock = provider->second.lock();
        ThrowIfNull(provider_lock, "invalidAuthorizationProviderReference");

        return provider_lock;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("service", service));
        return nullptr;
    }
}

}  // namespace authorization
}  // namespace engine
}  // namespace aace
