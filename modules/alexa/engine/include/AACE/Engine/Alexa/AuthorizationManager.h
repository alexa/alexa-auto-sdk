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

#ifndef AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_H
#define AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_H

#include <unordered_map>

#include <RegistrationManager/CustomerDataHandler.h>
#include <RegistrationManager/RegistrationManager.h>

#include "AuthorizationManagerInterface.h"
#include "AuthorizationManagerStorage.h"

namespace aace {
namespace engine {
namespace alexa {

class AuthorizationManager
        : public AuthorizationManagerInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface
        , public alexaClientSDK::registrationManager::CustomerDataHandler
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<AuthorizationManager> {
public:
    /**
     * Creates the reference of @c AuthorizationManager
     * 
     * @param storage Used for storing and retrieving the authorization adapter state
     * @param customerDataManager Component to register this component as a customer data container
     * @return On successful returns the reference to the @c AuthorizationManager otherwise returns the nullptr
     */
    static std::shared_ptr<AuthorizationManager> create(
        std::shared_ptr<AuthorizationManagerStorageInterface> storage,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager);

    /**
     * Get the reference to class that implements @c AuthDelegateInterface
     * 
     * @return Returns the valid reference pointer otherwise nullptr.
     */
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> getAuthInterface();

    /**
     * Set the registration manager reference.
     * This is used for carrying out the clean up using the @c logout API.
     * 
     * @param registrationManager Reference @c RegistrationManager used for performing logout process.
     */
    void setRegistrationManager(
        std::shared_ptr<alexaClientSDK::registrationManager::RegistrationManager> registrationManager);

    /// @name AuthorizationManagerInterface
    /// @{
    void registerAuthorizationAdapter(
        const std::string& service,
        std::shared_ptr<AuthorizationAdapterInterface> adapter) override;
    StartAuthorizationResult startAuthorization(const std::string& service) override;
    void authStateChanged(const std::string& service, State state, Error reason) override;
    bool logout(const std::string& service) override;
    /// @}

    /// @name alexaClientSDK::alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface
    /// @{
    void addAuthObserver(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) override;
    void removeAuthObserver(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) override;
    std::string getAuthToken() override;
    void onAuthFailure(const std::string& token) override;
    /// @}

    /// @name CustomerDataHandler
    /// @{
    void clearData() override;
    /// @}

protected:
    /// @name RequiresShutdown
    /// @{
    void doShutdown() override;
    /// @}

private:
    /**
     * Constructor
     * 
     * @param storage Stores and retrieves the active authorization from persistant storage.
     * @param customerDataManager Component to register this component as a customer data container
     */
    AuthorizationManager(
        std::shared_ptr<AuthorizationManagerStorageInterface> storage,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager);

    /**
     * Initializes the @c AuthorizationManager object
     */
    bool initialize();

    /**
     * Updates the internal auth state and notifies auth state to @c AuthObserverInterface 
     * observers.
     * 
     * @note This function to be called holding @c m_mutex lock.
     */
    void updateAuthStateAndNotifyAuthObserversLocked(State authState, Error authError);

    /**
     * Function to reset the internal auth state and perform logout process.
     * 
     * @note This function to be called holding @c m_mutex lock.
      * 
     * @return Returns @c false if @c m_registrationManager could not be locked, otherwise @c true.
     */
    bool resetAuthStateAndLogoutLocked();

    /**
     * Helper function to save the @c m_currentAdapterState to storage
     * Caller need to synchronize the access using @c m_mutex.
     * 
     * @return Returns @c true on successful storage, otherwise @c false.
     */
    bool saveCurrentAdapterStateLocked();

    /**
     * Clear the adapter state from storage.
     * 
     * @return Returns @c true on successful clearing the adapter state from the storage,
     * otherwise @c false.
     */
    bool clearAdapterState();

private:
    /// Alias for @c AdapterState
    using AdapterState = AuthorizationManagerStorage::AdapterState;

    /// Holds the registered authorization adapters
    std::unordered_map<std::string, std::shared_ptr<AuthorizationAdapterInterface>> m_serviceAndAuthorizationAdapterMap;

    /// Represents the current active authorization.
    AdapterState m_activeAdapterState;

    /// Current state of authorization. Access is synchronized with @c m_mutex.
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State m_authState;

    /// Current authorization error. Access is synchronized with @c m_mutex.
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error m_authError;

    /// Used for persisting the active authorization adapter name and state
    std::shared_ptr<AuthorizationManagerStorageInterface> m_storage;

    /// Reference to the @c RegistrationManager to carry out the logout process
    std::weak_ptr<alexaClientSDK::registrationManager::RegistrationManager> m_registrationManager;

    /// List of observers to be notified for the change in the auth state. Access is synchronized with @c m_mutex.
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface>>
        m_authDelegateObservers;

    /// To serialize the access to class API
    std::mutex m_mutex;

    /// To serialize the access to @c m_authDelegateObservers
    std::mutex m_authDelegateObserverMutex;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace
#endif  // AACE_ENGINE_ALEXA_AUTHORIZATION_MANAGER_H
