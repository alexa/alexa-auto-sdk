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

#include <sstream>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Authorization/AuthorizationEngineImpl.h>
#include <AACE/Engine/Utils/Metrics/Metrics.h>

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.authorization.AuthorizationEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AuthorizationEngineImpl";

/// Counter metrics for Authorization Platform APIs
static const std::string METRIC_AUTHORIZATION_EVENT_RECEIVED = "EventReceived";
static const std::string METRIC_AUTHORIZATION_AUTHORIZATION_STATE_CHANGED = "AuthorizationStateChanged";
static const std::string METRIC_AUTHORIZATION_AUTHORIZATION_ERROR = "AuthorizationError";
static const std::string METRIC_AUTHORIZATION_GET_AUTHORIZATION_DATA = "GetAuthorizationData";
static const std::string METRIC_AUTHORIZATION_SET_AUTHORIZATION_DATA = "SetAuthorizationData";
static const std::string METRIC_AUTHORIZATION_START_AUTHORIZATION = "StartAuthorization";
static const std::string METRIC_AUTHORIZATION_CANCEL_AUTHORIZATION = "CancelAuthorization";
static const std::string METRIC_AUTHORIZATION_SEND_EVENT = "SendEvent";
static const std::string METRIC_AUTHORIZATION_LOG_OUT = "Logout";

namespace aace {
namespace engine {
namespace authorization {

using namespace aace::engine::utils::metrics;

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
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onStartAuthorization", {METRIC_AUTHORIZATION_START_AUTHORIZATION, service});
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
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onCancelAuthorization", {METRIC_AUTHORIZATION_CANCEL_AUTHORIZATION, service});
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
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onSendEvent", {METRIC_AUTHORIZATION_SEND_EVENT, service});
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
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onLogout", {METRIC_AUTHORIZATION_LOG_OUT, service});
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
    std::stringstream authState;
    authState << state;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onAuthorizationStateChanged",
        {METRIC_AUTHORIZATION_AUTHORIZATION_STATE_CHANGED, service, authState.str()});
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
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onAuthorizationError", {METRIC_AUTHORIZATION_AUTHORIZATION_ERROR, service, error});
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(m_authorizationPlatformInterface, "nullAuthorizationPlatformInterface");
        m_authorizationPlatformInterface->authorizationError(service, error, message);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationEngineImpl::onEventReceived(const std::string& service, const std::string& event) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onEventReceived", {METRIC_AUTHORIZATION_EVENT_RECEIVED, service});
    AACE_DEBUG(LX(TAG));
    try {
        for (const auto& eventListener : m_eventListeners) {
            if (auto listener = eventListener.lock()) {
                listener->onEventReceived(service, event);
            }
        }

        ThrowIfNull(m_authorizationPlatformInterface, "nullAuthorizationPlatformInterface");
        m_authorizationPlatformInterface->eventReceived(service, event);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string AuthorizationEngineImpl::onGetAuthorizationData(const std::string& service, const std::string& key) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onGetAuthorizationData", {METRIC_AUTHORIZATION_GET_AUTHORIZATION_DATA, service});
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
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onSetAuthorizationData", {METRIC_AUTHORIZATION_SET_AUTHORIZATION_DATA, service});
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

void AuthorizationEngineImpl::addEventListener(std::shared_ptr<AuthorizationEventListenerInterface> eventListener) {
    m_eventListeners.push_back(eventListener);
}

void AuthorizationEngineImpl::removeEventListener(std::shared_ptr<AuthorizationEventListenerInterface> eventListener) {
    for (auto it = m_eventListeners.begin(); it != m_eventListeners.end(); ++it) {
        if (it->lock() == eventListener) {
            m_eventListeners.erase(it);
            return;
        }
    }
}

}  // namespace authorization
}  // namespace engine
}  // namespace aace
