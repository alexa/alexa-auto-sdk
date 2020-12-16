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

#include "AACE/Engine/Alexa/AuthorizationManager.h"

namespace aace {
namespace engine {
namespace alexa {

/// String to identify log entries originating from this file.
static const std::string TAG("AuthorizationManager");

using namespace alexaClientSDK::avsCommon::sdkInterfaces;

std::shared_ptr<AuthorizationManager> AuthorizationManager::create(
    std::shared_ptr<AuthorizationManagerStorageInterface> storage,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager) {
    try {
        ThrowIfNull(storage, "invalidStorage");
        ThrowIfNull(customerDataManager, "invalidCustomerDataManager");

        auto authorizationManager =
            std::shared_ptr<AuthorizationManager>(new AuthorizationManager(storage, customerDataManager));

        ThrowIfNot(authorizationManager->initialize(), "initializeFailed");

        return authorizationManager;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

AuthorizationManager::AuthorizationManager(
    std::shared_ptr<AuthorizationManagerStorageInterface> storage,
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager) :
        CustomerDataHandler{customerDataManager},
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_activeAdapterState{"", ""},
        m_authState{AuthObserverInterface::State::UNINITIALIZED},
        m_authError{AuthObserverInterface::Error::SUCCESS},
        m_storage(storage) {
}

bool AuthorizationManager::initialize() {
    try {
        AdapterState adapterState;
        ThrowIfNot(m_storage->loadAdapterState(adapterState), "loadAdapterStateFailed");
        m_activeAdapterState = adapterState;
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
    std::shared_ptr<alexaClientSDK::registrationManager::RegistrationManager> registrationManager) {
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
        ThrowIf(service.empty(), "emtpyService");

        std::lock_guard<std::mutex> lock(m_mutex);
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
        ThrowIf(service.empty(), "emtpyService");

        std::lock_guard<std::mutex> lock(m_mutex);
        ThrowIf(
            m_serviceAndAuthorizationAdapterMap.find(service) == m_serviceAndAuthorizationAdapterMap.end(),
            "serviceNotRegistered");

        if (m_activeAdapterState.first.empty()) {
            m_activeAdapterState = {service, "NOT_AUTHORIZED"};
            ThrowIfNot(saveCurrentAdapterStateLocked(), "saveCurrentAdapterStateLockedFailed");
            return StartAuthorizationResult::AUTHORIZE;
        } else {
            if (m_activeAdapterState.first == service) {
                return StartAuthorizationResult::REAUTHORIZE;
            } else {
                ThrowIfNot(resetAuthStateAndLogoutLocked(), "resetAuthStateAndLogoutFailed");
                m_activeAdapterState = {service, "NOT_AUTHORIZED"};

                ThrowIfNot(saveCurrentAdapterStateLocked(), "saveCurrentAdapterStateLockedFailed");

                return StartAuthorizationResult::AUTHORIZE;
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return StartAuthorizationResult::FAILED;
    }
}

void AuthorizationManager::updateAuthStateAndNotifyAuthObserversLocked(State authState, Error authError) {
    AACE_DEBUG(LX(TAG));
    try {
        m_authState = authState;
        m_authError = authError;

        std::unique_lock<std::mutex> lock(m_authDelegateObserverMutex);
        auto copyOfObservers = m_authDelegateObservers;
        lock.unlock();

        for (auto& observer : copyOfObservers) {
            observer->onAuthStateChange(m_authState, m_authError);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AuthorizationManager::resetAuthStateAndLogoutLocked() {
    AACE_DEBUG(LX(TAG));
    try {
        updateAuthStateAndNotifyAuthObserversLocked(State::UNINITIALIZED, Error::SUCCESS);
        auto m_registrationManager_lock = m_registrationManager.lock();
        ThrowIfNull(m_registrationManager_lock, "invalidRegistrationManagerReference");
        m_registrationManager_lock->logout();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AuthorizationManager::saveCurrentAdapterStateLocked() {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNot(m_storage->saveAdapterState(m_activeAdapterState), "saveAdapterStateFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AuthorizationManager::clearAdapterState() {
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
        ThrowIf(service.empty(), "emtpyService");
        ThrowIf(m_activeAdapterState.first.empty(), "noActiveAuthorization");

        std::lock_guard<std::mutex> lock(m_mutex);
        if (service == m_activeAdapterState.first) {
            if (state == State::REFRESHED && m_activeAdapterState.second == "NOT_AUTHORIZED") {
                m_activeAdapterState = {m_activeAdapterState.first, "AUTHORIZED"};
                ThrowIfNot(saveCurrentAdapterStateLocked(), "saveCurrentAdapterStateLockedFailed");
            }
            updateAuthStateAndNotifyAuthObserversLocked(state, reason);
        } else {
            AACE_ERROR(LX(TAG).m("notActiveService").d("service", service));
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AuthorizationManager::logout(const std::string& service) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(service.empty(), "emtpyService");
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_serviceAndAuthorizationAdapterMap.find(service) == m_serviceAndAuthorizationAdapterMap.end()) {
            Throw("serviceNotRegistered");
        }

        // Check if the service is the previously used service.
        if (m_activeAdapterState.first == service) {
            ThrowIfNot(resetAuthStateAndLogoutLocked(), "resetAuthStateAndLogoutFailed");
            return true;
        }
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

// clearData() could get called in 3 scenarios. 1) starting a new authorization that results in
// previous authorization to be logged out, 2) explict logout() call, or 3) direct call to the
// registration manager to log out.  Also, this function does not acquire the mutex as this
// will be called on registration manager logout() function thread.
void AuthorizationManager::clearData() {
    AACE_DEBUG(LX(TAG));
    try {
        if (m_serviceAndAuthorizationAdapterMap.find(m_activeAdapterState.first) ==
            m_serviceAndAuthorizationAdapterMap.end()) {
            AACE_WARN(LX(TAG).m("adapterNotRegistered").d("service", m_activeAdapterState.first));
        } else {
            m_serviceAndAuthorizationAdapterMap[m_activeAdapterState.first]->deregister();
        }
        m_activeAdapterState = {"", ""};
        ThrowIfNot(clearAdapterState(), "clearAdapterStateFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
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

std::string AuthorizationManager::getAuthToken() {
    try {
        ThrowIf(m_activeAdapterState.first.empty(), "NoActiveAuthorization");
        return m_serviceAndAuthorizationAdapterMap[m_activeAdapterState.first]->getAuthToken();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

void AuthorizationManager::onAuthFailure(const std::string& token) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(m_activeAdapterState.first.empty(), "NoActiveAuthorization");
        m_serviceAndAuthorizationAdapterMap[m_activeAdapterState.first]->onAuthFailure(token);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthorizationManager::doShutdown() {
    AACE_DEBUG(LX(TAG));
    // The mutex makes sure that shutdown and logout do not happen at the
    // same time
    std::unique_lock<std::mutex> lock(m_mutex);
    m_registrationManager.reset();
    m_authDelegateObservers.clear();
    m_storage.reset();
    m_serviceAndAuthorizationAdapterMap.clear();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
