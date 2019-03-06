/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CBL_CBL_AUTH_DELEGATE_H
#define AACE_ENGINE_CBL_CBL_AUTH_DELEGATE_H

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <atomic>

#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPost.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/Utils/RetryTimer.h>
#include <RegistrationManager/CustomerDataHandler.h>

#include "CBLAuthDelegateConfiguration.h"
#include "CBLAuthRequesterInterface.h"

namespace aace {
namespace engine {
namespace cbl {

class CBLAuthDelegate
        : public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface
        , public alexaClientSDK::registrationManager::CustomerDataHandler {
public:
    static std::shared_ptr<CBLAuthDelegate> create(
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
        std::shared_ptr<CBLAuthDelegateConfiguration> configuration,
        std::shared_ptr<CBLAuthRequesterInterface> cblAuthRequester );

    ~CBLAuthDelegate();

    // AuthDelegateInterface
    void addAuthObserver(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) override;
    void removeAuthObserver(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) override;
    std::string getAuthToken() override;
    void onAuthFailure(const std::string& token) override;

    // CustomerDataHandler
    void clearData() override;

    void start( bool onStart );
    void cancel();

private:
    enum class FlowState {
        STARTING,
        REQUESTING_CODE_PAIR,
        REQUESTING_TOKEN,
        REFRESHING_TOKEN,
        STOPPING
    };

    CBLAuthDelegate(
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
        std::shared_ptr<CBLAuthDelegateConfiguration> configuration,
        std::shared_ptr<CBLAuthRequesterInterface> cblAuthRequester,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPost> httpPost );


    bool initialize();
    void stop();
    void handleAuthorizationFlow();

    FlowState handleStarting();
    FlowState handleRequestingCodePair();
    FlowState handleRequestingToken();
    FlowState handleRefreshingToken();
    FlowState handleStopping();

    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse requestCodePair();
    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse requestToken();
    alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse requestRefresh();

    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error receiveCodePairResponse(
        const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response );

    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error receiveTokenResponse(
        const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response,
        bool expiresImmediately);

    void setAuthState( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State newState );
    void setAuthError( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error );

    void setRefreshToken( const std::string& refreshToken );
    void clearRefreshToken();

    bool isStopping();

    std::shared_ptr<alexaClientSDK::avsCommon::utils::libcurlUtils::HttpPostInterface> m_httpPost;
    std::shared_ptr<CBLAuthRequesterInterface> m_cblAuthRequester;
    std::shared_ptr<CBLAuthDelegateConfiguration> m_configuration;

    bool m_isStopping;
    std::mutex m_mutex;
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface>> m_observers;

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

    aace::engine::cbl::CBLAuthRequesterInterface::CBLStateChangedReason m_stateChangeReason;
};

} // aace::engine::cbl
} // aace::engine
} // aace

#endif
