/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_ALEXA_ENGINE_LOCATION_STATE_PROVIDER_H
#define AACE_ENGINE_ALEXA_ALEXA_ENGINE_LOCATION_STATE_PROVIDER_H

#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/StateProviderInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include <AACE/Engine/Location/LocationServiceObserverInterface.h>
#include <AACE/Engine/Location/LocationServiceInterface.h>
#include "GeolocationServiceInterface.h"

namespace aace {
namespace engine {
namespace alexa {
class AlexaEngineLocationStateProvider
        : public alexaClientSDK::avsCommon::sdkInterfaces::StateProviderInterface
        , public aace::engine::location::LocationServiceObserverInterface
        , public GeolocationServiceInterface
        , public std::enable_shared_from_this<AlexaEngineLocationStateProvider> {
private:
    AlexaEngineLocationStateProvider(
        std::shared_ptr<aace::engine::location::LocationServiceInterface> locationProvider,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

public:
    static std::shared_ptr<AlexaEngineLocationStateProvider> create(
        std::shared_ptr<aace::engine::location::LocationServiceInterface> locationProvider,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    /// @name StateProviderInterface functions
    /// @{
    void provideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        const unsigned int stateRequestToken) override;
    /// @}

    /// @name LocationServiceObserverInterface functions
    /// @{
    void onLocationServiceAccessChanged(LocationServiceAccess access) override;
    /// @}

    /// @name GeolocationServiceInterface functions
    /// @{
    void shutdown() override;
    /// @}

private:
    void executeProvideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        const unsigned int stateRequestToken);
    void initialize();

private:
    aace::location::LocationProvider::LocationServiceAccess m_locationServiceAccess;
    std::shared_ptr<aace::engine::location::LocationServiceInterface> m_locationProvider;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
    std::mutex m_locationServiceAccessMutex;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  //AACE_ENGINE_ALEXA_ALEXA_ENGINE_LOCATION_STATE_PROVIDER_H
