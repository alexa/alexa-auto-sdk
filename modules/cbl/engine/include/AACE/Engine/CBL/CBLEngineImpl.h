/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <memory>
#include <string>

#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include <AACE/CBL/CBL.h>
#include <AACE/CBL/CBLEngineInterface.h>

#include <AACE/Engine/Alexa/AlexaEndpointInterface.h>
#include <AACE/Engine/Alexa/LocaleAssetsManager.h>
#include <AACE/Engine/Authorization/AuthorizationProviderListenerInterface.h>
#include <AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h>

#include "CBLAuthorizationProvider.h"
#include "CBLConfiguration.h"

namespace aace {
namespace engine {
namespace cbl {

class CBLEngineImpl
        : public aace::cbl::CBLEngineInterface
        , public authorization::AuthorizationProviderListenerInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<CBLEngineImpl> {
private:
    CBLEngineImpl(std::shared_ptr<aace::cbl::CBL> cblPlatformInterface);

    bool initialize(
        std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::chrono::seconds codePairRequestTimeout,
        std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
        std::weak_ptr<aace::engine::alexa::LocaleAssetsManager> localeAssetManager,
        std::shared_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> propertyManager,
        bool enableUserProfile);

public:
    static std::shared_ptr<CBLEngineImpl> create(
        std::shared_ptr<aace::cbl::CBL> cblPlatformInterface,
        std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::chrono::seconds codePairRequestTimeout,
        std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
        std::weak_ptr<aace::engine::alexa::LocaleAssetsManager> localeAssetManager,
        std::shared_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> propertyManager,
        bool enableUserProfile);

    void enable();
    void disable();

    /// @name AuthorizationProviderListenerInterface
    /// @{
    void onAuthorizationStateChanged(
        const std::string& service,
        AuthorizationProviderListenerInterface::AuthorizationState state) override;
    void onAuthorizationError(const std::string& service, const std::string& error, const std::string& message)
        override;
    void onEventReceived(const std::string& service, const std::string& request) override;
    std::string onGetAuthorizationData(const std::string& service, const std::string& key) override;
    void onSetAuthorizationData(const std::string& service, const std::string& key, const std::string& data) override;
    /// @}

    // CBLEngineInterface
    void onStart() override;
    void onCancel() override;
    void onReset() override;

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::cbl::CBL> m_cblPlatformInterface;
    std::shared_ptr<CBLAuthorizationProvider> m_cblAuthorizationProvider;

    /// Represents the possible states of AuthProviderEngineImpl
    enum class CBLEngineState {
        /// Initial State
        INITIALIZED,

        /// Same as AuthorizationState::AUTHORIZING
        AUTHORIZING,

        /// Same as AuthorizationState::AUTHORIZED
        AUTHORIZED,

        /// Same as AuthorizationState::UNAUTHORIZED
        UNAUTHORIZED,

        /// Represents the error when onAuthorizationError() occurs
        ERROR
    };

    /// Represents the current state of CBLEngineImpl
    CBLEngineState m_state;

    /// Synchronizes @c CBLEngineInterface calls with @c AuthorizationProviderListenerInterface callbacks that update @c m_state
    std::condition_variable m_cv;

    /// To serialize access to @c m_state
    std::mutex m_mutex;
};

}  // namespace cbl
}  // namespace engine
}  // namespace aace

#endif
