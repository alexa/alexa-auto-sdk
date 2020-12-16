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

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Authorization/AuthorizationEngineImpl.h>

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.authorization.AuthorizationEngineImpl");

namespace aace {
namespace engine {
namespace authorization {

AuthorizationEngineImpl::AuthorizationEngineImpl(
    std::shared_ptr<aace::authorization::Authorization> authorizationPlatformInterface,
    std::shared_ptr<AuthorizationServiceInterface> authorizationService) :
        m_authorizationPlatformInterface(authorizationPlatformInterface), m_authorizationService(authorizationService) {
}

std::shared_ptr<AuthorizationEngineImpl> AuthorizationEngineImpl::create(
    std::shared_ptr<aace::authorization::Authorization> authorizationPlatformInterface,
    std::shared_ptr<AuthorizationServiceInterface> authorizationService) {
    try {
        ThrowIfNull(authorizationPlatformInterface, "invalidAuthorizationPlatformInterface");
        ThrowIfNull(authorizationService, "invalidAuthorizationServiceInterface");

        auto authorizationEngineImpl = std::shared_ptr<AuthorizationEngineImpl>(
            new AuthorizationEngineImpl(authorizationPlatformInterface, authorizationService));

        // set the platform engine interface reference
        authorizationPlatformInterface->setEngineInterface(authorizationEngineImpl);

        return authorizationEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void AuthorizationEngineImpl::onStartAuthorization(const std::string& service, const std::string& data) {
    AACE_DEBUG(LX(TAG).d("service", service).sensitive("data", data));
    try {
        ThrowIf(service.empty(), "invalidService");

        auto provider = m_authorizationService->getProvider(service);
        ThrowIfNull(provider, "serviceNotFound");

        ThrowIfNot(provider->startAuthorization(data), "startAuthorizationFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationEngineImpl::onCancelAuthorization(const std::string& service) {
    AACE_DEBUG(LX(TAG).d("service", service));
    try {
        ThrowIf(service.empty(), "invalidService");

        auto provider = m_authorizationService->getProvider(service);
        ThrowIfNull(provider, "serviceNotFound");

        ThrowIfNot(provider->cancelAuthorization(), "cancelAuthorizationFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationEngineImpl::onSendEvent(const std::string& service, const std::string& event) {
    AACE_DEBUG(LX(TAG).d("service", service).sensitive("event", event));
    try {
        ThrowIf(service.empty(), "invalidService");

        auto provider = m_authorizationService->getProvider(service);
        ThrowIfNull(provider, "serviceNotFound");

        ThrowIfNot(provider->sendEvent(event), "sendEventFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationEngineImpl::onLogout(const std::string& service) {
    AACE_DEBUG(LX(TAG).d("service", service));
    try {
        ThrowIf(service.empty(), "invalidService");

        auto provider = m_authorizationService->getProvider(service);
        ThrowIfNull(provider, "serviceNotFound");

        ThrowIfNot(provider->logout(), "logoutFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationEngineImpl::onAuthorizationStateChanged(
    const std::string& service,
    AuthorizationProviderListenerInterface::AuthorizationState state) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(m_authorizationPlatformInterface, "nullAuthorizationPlatformInterface");
        m_authorizationPlatformInterface->authorizationStateChanged(service, state);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationEngineImpl::onAuthorizationError(
    const std::string& service,
    const std::string& error,
    const std::string& message) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(m_authorizationPlatformInterface, "nullAuthorizationPlatformInterface");
        m_authorizationPlatformInterface->authorizationError(service, error, message);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationEngineImpl::onEventReceived(const std::string& service, const std::string& event) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(m_authorizationPlatformInterface, "nullAuthorizationPlatformInterface");
        m_authorizationPlatformInterface->eventReceived(service, event);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string AuthorizationEngineImpl::onGetAuthorizationData(const std::string& service, const std::string& key) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(m_authorizationPlatformInterface, "nullAuthorizationPlatformInterface");
        return m_authorizationPlatformInterface->getAuthorizationData(service, key);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

void AuthorizationEngineImpl::onSetAuthorizationData(
    const std::string& service,
    const std::string& key,
    const std::string& data) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(m_authorizationPlatformInterface, "nullAuthorizationPlatformInterface");
        m_authorizationPlatformInterface->setAuthorizationData(service, key, data);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationEngineImpl::doShutDown() {
    if (m_authorizationPlatformInterface != nullptr) {
        m_authorizationPlatformInterface->setEngineInterface(nullptr);
        m_authorizationPlatformInterface.reset();
    }
}

}  // namespace authorization
}  // namespace engine
}  // namespace aace
