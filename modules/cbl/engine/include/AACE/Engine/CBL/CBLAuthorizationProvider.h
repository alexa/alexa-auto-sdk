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

#ifndef AACE_ENGINE_CBL_CBL_AUTHORIZATION_PROVIDER_H
#define AACE_ENGINE_CBL_CBL_AUTHORIZATION_PROVIDER_H

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <atomic>

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpGet.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPost.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include <AACE/Engine/Authorization/AuthorizationProvider.h>
#include <AACE/Engine/Alexa/AuthorizationAdapterInterface.h>
#include <AACE/Engine/Alexa/AuthorizationManagerInterface.h>
#include <AACE/Engine/PropertyManager/PropertyListenerInterface.h>
#include <AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h>
#include <AACE/Engine/Utils/Threading/Executor.h>

#include "CBLConfigurationInterface.h"

namespace aace {
namespace engine {
namespace cbl {

class CBLAuthorizationProvider
        : public aace::engine::authorization::AuthorizationProvider
        , public aace::engine::alexa::AuthorizationAdapterInterface
        , public aace::engine::propertyManager::PropertyListenerInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<CBLAuthorizationProvider> {
private:
    /**
     * Constructor.
     */
    CBLAuthorizationProvider(
        const std::string& service,
        std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface,
        std::shared_ptr<CBLConfigurationInterface> configuration,
        bool enableUserProfile);

    /**
     * Initializes the object.
     */
    bool initialize(std::shared_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> propertyManager);

public:
    static std::shared_ptr<CBLAuthorizationProvider> create(
        const std::string& service,
        std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface,
        std::shared_ptr<CBLConfigurationInterface> configuration,
        std::shared_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> propertyManager = nullptr,
        bool enableUserProfile = false);

    /// @name AuthorizationProvider
    /// @{
    bool startAuthorization(const std::string& data) override;
    bool cancelAuthorization() override;
    bool logout() override;
    bool sendEvent(const std::string& payload) override;
    /// @}

    /// @name AuthorizationAdapterInterface
    /// @{
    void deregister() override;
    std::string getAuthToken() override;
    void onAuthFailure(const std::string& token) override;
    /// @}

    /// @name PropertyListenerInterface
    /// @{
    void propertyChanged(const std::string& name, const std::string& newValue) override;
    /// @}

    /**
     * @internal
     * For starting the authorization.
     * Note: To be used only by CBL legacy implementation.
     *
     * @param data The Authorization data
     * @param explicitStart Indicates an explict call to @c CBL::start()
     */
    void startAuthorizationLegacy(const std::string& data, bool explicitStart);

protected:
    /// @name RequiresShutdown
    /// @{
    void doShutdown() override;
    /// @}

private:
    // Alias for readability
    using AuthorizationState = authorization::AuthorizationProviderListenerInterface::AuthorizationState;

    enum class FlowState { STARTING, REQUESTING_CODE_PAIR, REQUESTING_TOKEN, REFRESHING_TOKEN, STOPPING };

    void stopAuthFlowThread(bool resetData, bool notifyAuthStateChange = true);

    void handleAuthorizationFlow();
    void handleRequestingUserProfile();

    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse doPost(
        const std::string& url,
        const std::vector<std::string> headerLines,
        const std::vector<std::pair<std::string, std::string>>& data,
        std::chrono::seconds timeout);

    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse doGet(
        const std::string& url,
        const std::vector<std::string>& headers);

    FlowState handleStarting();
    FlowState handleRequestingCodePair();
    FlowState handleRequestingToken();
    FlowState handleRefreshingToken();
    FlowState handleStopping();

    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse requestCodePair();
    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse requestToken();
    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse requestRefresh();
    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse requestUserProfile();

    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error receiveCodePairResponse(
        const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response);

    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error receiveTokenResponse(
        const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response,
        bool expiresImmediately);

    void setAuthState(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State newState,
        bool notifyAuthStateChange = true);
    void setAuthError(alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error);

    void setRefreshToken(const std::string& refreshToken);
    void clearRefreshToken();

    bool isStopping();

    std::shared_ptr<CBLConfigurationInterface> m_configuration;

    bool m_isStopping;
    bool m_authFailureReported;
    bool m_explicitAuthorizationRequest;

    /// Represents the call to `CBL::start()` to start the authorization
    bool m_legacyCBLExplicitStart;

    std::string m_accessToken;
    std::string m_refreshToken;
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State m_authState;
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error m_authError;

    std::string m_deviceCode;
    std::string m_userCode;

    std::thread m_authorizationFlowThread;
    std::chrono::steady_clock::time_point m_codePairExpirationTime;
    std::chrono::steady_clock::time_point m_tokenExpirationTime;
    std::chrono::steady_clock::time_point m_timeToRefresh;
    std::chrono::steady_clock::time_point m_requestTime;

    std::condition_variable m_wake;
    int m_retryCount;
    bool m_newRefreshToken;
    std::atomic<bool> m_threadActive;

    FlowState m_flowState;
    std::string m_stateChangeReason;
    bool m_enableUserProfile;
    std::string m_scope;
    std::string m_locale;
    std::mutex m_localeMutex;

    /// Holds the service name
    std::string m_service;

    /// Holds the current authorization state.
    AuthorizationState m_currentAuthState;

    /// Reference to access the authorization manager interface
    std::weak_ptr<aace::engine::alexa::AuthorizationManagerInterface> m_authorizationManager;

    /// To serialize the access to this module API
    std::mutex m_mutex;

    /// This is the worker thread for the @c CBLAuthorizationProvider.
    aace::engine::utils::threading::Executor m_executor;
};

}  // namespace cbl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CBL_CBL_AUTHORIZATION_PROVIDER_H
