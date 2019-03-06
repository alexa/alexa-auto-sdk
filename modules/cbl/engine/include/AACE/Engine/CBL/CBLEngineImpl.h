/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CBL_CBL_ENGINE_IMPL_H
#define AACE_ENGINE_CBL_CBL_ENGINE_IMPL_H

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>

#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <RegistrationManager/CustomerDataHandler.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include <AACE/CBL/CBL.h>
#include <AACE/CBL/CBLEngineInterface.h>
#include "CBLAuthDelegate.h"
#include "CBLAuthRequesterInterface.h"
#include "CBLAuthDelegateConfiguration.h"

namespace aace {
namespace engine {
namespace cbl {

class CBLEngineImpl :
    public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface,
    public CBLAuthRequesterInterface,
    public aace::cbl::CBLEngineInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<CBLEngineImpl> {
    
private:
    CBLEngineImpl( std::shared_ptr<aace::cbl::CBL> cblPlatformInterface );

    bool initialize(
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::chrono::seconds codePairRequestTimeout,
        const std::string& endpoint );

public:
    static std::shared_ptr<CBLEngineImpl> create(
        std::shared_ptr<aace::cbl::CBL> cblPlatformInterface,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::chrono::seconds codePairRequestTimeout,
        const std::string& endpoint );

    void engineStart();

    // AuthDelegateInterface
    void addAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer ) override;
    void removeAuthObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer ) override;
    std::string getAuthToken() override;
    void onAuthFailure(const std::string& token) override;

    // CBLAuthRequestInterface
    void cblStateChanged( CBLAuthRequesterInterface::CBLState state, CBLAuthRequesterInterface::CBLStateChangedReason reason, const std::string& url, const std::string& code ) override;
    void clearRefreshToken() override;
    void setRefreshToken( const std::string& refreshToken ) override;
    std::string getRefreshToken() override;

    // CBLEngineInterface
    void onStart() override;
    void onCancel() override;

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::cbl::CBL> m_cblPlatformInterface;
    std::shared_ptr<CBLAuthDelegate> m_cblAuthDelegate;
};

} // aace::engine::cbl
} // aace::engine
} // aace

#endif
