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

#include <AACE/Engine/Core/EngineMacros.h>

#include "AACE/Engine/Alexa/AuthorizationManager.h"

namespace aace {
namespace engine {
namespace alexa {

/// String to identify log entries originating from this file.
static const std::string TAG("AuthorizationManager");

using namespace alexaClientSDK::avsCommon::sdkInterfaces;

std::shared_ptr<AuthorizationManager> AuthorizationManager::create(
    std::shared_ptr<AuthorizationManagerStorageInterface> storage) {
    try {
        ThrowIfNull(storage, "invalidStorage");

        auto authorizationManager = std::shared_ptr<AuthorizationManager>(new AuthorizationManager(storage));

        ThrowIfNot(authorizationManager->initialize(), "initializeFailed");

        return authorizationManager;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

AuthorizationManager::AuthorizationManager(std::shared_ptr<AuthorizationManagerStorageInterface> storage) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_activeAdapter{"", ""},
        m_authState{AuthObserverInterface::State::UNINITIALIZED},
        m_authError{AuthObserverInterface::Error::SUCCESS},
        m_storage(storage) {
}

bool AuthorizationManager::initialize() {
    try {
        AdapterState adapterState;
        ThrowIfNot(m_storage->loadAdapterState(adapterState), "loadAdapterStateFailed");
        m_activeAdapter = adapterState;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> AuthorizationManager::
    getAuthInterface() {
    AACE_DEBUG(LX(TAG));
    return (shared_from_this());
}

void AuthorizationManager::setRegistrationManager(
    std::shared_ptr<alexaClientSDK::registrationManager::RegistrationManagerInterface> registrationManager) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(registrationManager, "invalidRegistrationManager");
        m_registrationManager = registrationManager;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationManager::registerAuthorizationAdapter(
    const std::string& service,
    std::shared_ptr<AuthorizationAdapterInterface> adapter) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(adapter, "invalidAuthorizationAdapter");
        ThrowIf(service.empty(), "emptyService");

        std::lock_guard<std::mutex> lock(m_callMutex);
        if (m_serviceAndAuthorizationAdapterMap.find(service) != m_serviceAndAuthorizationAdapterMap.end()) {
            AACE_WARN(LX(TAG).m("replacingAlreadyRegisteredAdapter").d("service", service));
        }

        m_serviceAndAuthorizationAdapterMap[service] = adapter;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

AuthorizationManager::StartAuthorizationResult AuthorizationManager::startAuthorization(const std::string& service) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(service.empty(), "emptyService");
        ThrowIf(
            m_serviceAndAuthorizationAdapterMap.find(service) == m_serviceAndAuthorizationAdapterMap.end(),
            "serviceNotRegistered");

        std::lock_guard<std::mutex> lock(m_callMutex);
        std::unique_lock<std::mutex> activeAdapterLock(m_activeAdapterMutex);

        if (m_activeAdapter.first.empty()) {
            m_activeAdapter = {service, "NOT_AUTHORIZED"};
            ThrowIfNot(saveCurrentAdapterStateLocked(), "saveCurrentAdapterStateLockedFailed");
            return StartAuthorizationResult::AUTHORIZE;
        } else {
            if (m_activeAdapter.first == service) {
                return StartAuthorizationResult::REAUTHORIZE;
            } else {
                // Unlock the activeAdapterLock function of AuthDelegateInterface could get called.
                activeAdapterLock.unlock();
                updateAuthStateAndNotifyAuthObservers(State::UNINITIALIZED, Error::SUCCESS);
                ThrowIfNot(performLogoutLocked(), "performLogoutLockedFailed");

                // Lock activeAdapterLock, as we are using m_activeAdapter
                activeAdapterLock.lock();
                performDeregisterLocked();
                m_activeAdapter = {service, "NOT_AUTHORIZED"};
                ThrowIfNot(saveCurrentAdapterStateLocked(), "saveCurrentAdapterStateLockedFailed");

                return StartAuthorizationResult::AUTHORIZE;
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return StartAuthorizationResult::FAILED;
    }
}

void AuthorizationManager::updateAuthStateAndNotifyAuthObservers(State authState, Error authError) {
    AACE_DEBUG(LX(TAG));
    try {
        m_authState = authState;
        m_authError = authError;

        std::unique_lock<std::mutex> lock(m_authDelegateObserverMutex);
        auto copyOfObservers = m_authDelegateObservers;
        lock.unlock();

        // Enable the metrics emitter module first when the auth state changes to REFRESHED, so that
        // it could accept the metrics that could be emitted immediately by the auth delegate observers.
        std::unique_lock<std::mutex> metricsLock(m_metricEmissionListenerMutex);
        if (m_authState == State::REFRESHED && m_metricsEmissionListener) {
            m_metricsEmissionListener->onMetricEmissionStateChanged(true);
        }
        metricsLock.unlock();

        for (auto& observer : copyOfObservers) {
            observer->onAuthStateChange(m_authState, m_authError);
        }

        // Disable the metrics emitter module only after auth delegate observers are notified about the
        // auth state as UNINITIALIZED. This allows any metrics from the auth observer to be emitted and
        // are not dropped.
        metricsLock.lock();
        if (m_authState == State::UNINITIALIZED && m_metricsEmissionListener) {
            m_metricsEmissionListener->onMetricEmissionStateChanged(false);
        }
        metricsLock.unlock();

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AuthorizationManager::performLogoutLocked() {
    AACE_DEBUG(LX(TAG));
    try {
        auto m_registrationManager_lock = m_registrationManager.lock();
        ThrowIfNull(m_registrationManager_lock, "invalidRegistrationManagerReference");
        m_registrationManager_lock->logout();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AuthorizationManager::performDeregisterLocked() {
    AACE_DEBUG(LX(TAG));
    // It is possible that previously active authorization adapter may not be registered after
    // Engine stop and start. So, check before calling deregister()
    if (m_serviceAndAuthorizationAdapterMap.find(m_activeAdapter.first) != m_serviceAndAuthorizationAdapterMap.end()) {
        m_serviceAndAuthorizationAdapterMap.at(m_activeAdapter.first)->deregister();
    } else {
        AACE_WARN(LX(TAG).m("serviceNotFound").d("service", m_activeAdapter.first));
    }
}

bool AuthorizationManager::saveCurrentAdapterStateLocked() {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNot(m_storage->saveAdapterState(m_activeAdapter), "saveAdapterStateFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AuthorizationManager::clearAdapterStateLocked() {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNot(m_storage->clearAdapterStateTable(), "clearAdapterStateFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AuthorizationManager::authStateChanged(const std::string& service, State state, Error reason) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(service.empty(), "emptyService");
        ThrowIf(m_activeAdapter.first.empty(), "noActiveAuthorization");

        std::unique_lock<std::mutex> lock(m_callMutex);
        std::unique_lock<std::mutex> activeAdapterLock(m_activeAdapterMutex);
        if (m_serviceAndAuthorizationAdapterMap.find(service) == m_serviceAndAuthorizationAdapterMap.end()) {
            Throw("serviceNotRegistered");
        }
        if (service == m_activeAdapter.first) {
            if (state == State::REFRESHED && m_activeAdapter.second == "NOT_AUTHORIZED") {
                m_activeAdapter = {m_activeAdapter.first, "AUTHORIZED"};
                ThrowIfNot(saveCurrentAdapterStateLocked(), "saveCurrentAdapterStateLockedFailed");
            }
            // Unlock the activeAdapterLock function of AuthDelegateInterface could get called.
            activeAdapterLock.unlock();
            updateAuthStateAndNotifyAuthObservers(state, reason);
        } else {
            Throw("notActiveService");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("service", service));
    }
}

bool AuthorizationManager::logout(const std::string& service) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(service.empty(), "emptyService");
        ThrowIf(
            m_serviceAndAuthorizationAdapterMap.find(service) == m_serviceAndAuthorizationAdapterMap.end(),
            "serviceNotRegistered");

        std::lock_guard<std::mutex> lock(m_callMutex);
        std::unique_lock<std::mutex> activeAdapterLock(m_activeAdapterMutex);

        // Check if the service is previously active adapter
        if (m_activeAdapter.first == service) {
            // Unlock the activeAdapterLock function of AuthDelegateInterface could get called.
            activeAdapterLock.unlock();
            updateAuthStateAndNotifyAuthObservers(State::UNINITIALIZED, Error::SUCCESS);
            ThrowIfNot(performLogoutLocked(), "performLogoutLockedFailed");

            // Lock activeAdapterLock, as we are using m_activeAdapter
            activeAdapterLock.lock();
            performDeregisterLocked();
            m_activeAdapter = {"", ""};
            ThrowIfNot(clearAdapterStateLocked(), "clearAdapterStateFailed");

            return true;
        } else {
            Throw("notActiveService");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("service", service));
        return false;
    }
}

void AuthorizationManager::addAuthObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) {
    try {
        std::lock_guard<std::mutex> lock(m_authDelegateObserverMutex);
        ThrowIfNull(observer, "nullObserver");
        ThrowIfNot(m_authDelegateObservers.insert(observer).second, "observerAlreadyAdded");
        observer->onAuthStateChange(m_authState, m_authError);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationManager::removeAuthObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) {
    try {
        std::lock_guard<std::mutex> lock(m_authDelegateObserverMutex);
        ThrowIfNull(observer, "nullObserver");
        ThrowIf(m_authDelegateObservers.erase(observer), "observerNotAdded");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationManager::addListener(std::shared_ptr<MetricsEmissionListenerInterface> listener) {
    try {
        std::unique_lock<std::mutex> lock(m_metricEmissionListenerMutex);
        ThrowIfNull(listener, "nullListener");
        m_metricsEmissionListener = listener;
        lock.unlock();

        m_metricsEmissionListener->onMetricEmissionStateChanged(m_authState == State::REFRESHED);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationManager::removeListener(std::shared_ptr<MetricsEmissionListenerInterface> listener) {
    try {
        std::lock_guard<std::mutex> lock(m_metricEmissionListenerMutex);
        ThrowIfNull(listener, "nullListener");
        ThrowIfNot(m_metricsEmissionListener == listener, "invalidListenerReceived");
        m_metricsEmissionListener.reset();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string AuthorizationManager::getAuthToken() {
    try {
        std::lock_guard<std::mutex> lock(m_activeAdapterMutex);
        ThrowIf(m_activeAdapter.first.empty(), "noActiveAuthorization");
        // Shutdown may have be called or authorization adapter is not registered
        ThrowIf(
            m_serviceAndAuthorizationAdapterMap.find(m_activeAdapter.first) ==
                m_serviceAndAuthorizationAdapterMap.end(),
            "adapterNotRegistered");

        return m_serviceAndAuthorizationAdapterMap.at(m_activeAdapter.first)->getAuthToken();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("activeAdapter", m_activeAdapter.first));
        return "";
    }
}

void AuthorizationManager::onAuthFailure(const std::string& token) {
    AACE_DEBUG(LX(TAG));
    try {
        std::lock_guard<std::mutex> lock(m_activeAdapterMutex);
        ThrowIf(m_activeAdapter.first.empty(), "noActiveAuthorization");
        // Shutdown may have be called or authorization adapter is not registered
        ThrowIf(
            m_serviceAndAuthorizationAdapterMap.find(m_activeAdapter.first) ==
                m_serviceAndAuthorizationAdapterMap.end(),
            "adapterNotRegistered");

        m_serviceAndAuthorizationAdapterMap.at(m_activeAdapter.first)->onAuthFailure(token);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("activeAdapter", m_activeAdapter.first));
    }
}

void AuthorizationManager::doShutdown() {
    AACE_DEBUG(LX(TAG));
    std::unique_lock<std::mutex> lock(m_callMutex);
    m_registrationManager.reset();
    m_authDelegateObservers.clear();
    m_storage.reset();
    m_serviceAndAuthorizationAdapterMap.clear();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
