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

#ifndef AACE_ENGINE_CBL_CBL_CONFIGURATION_H
#define AACE_ENGINE_CBL_CBL_CONFIGURATION_H

#include <chrono>
#include <memory>
#include <string>

#include <AVSCommon/Utils/DeviceInfo.h>

#include "AACE/Engine/Alexa/AlexaEndpointInterface.h"
#include "AACE/Engine/Alexa/LocaleAssetsManager.h"

#include "CBLConfigurationInterface.h"

namespace aace {
namespace engine {
namespace cbl {

class CBLConfiguration : public CBLConfigurationInterface {
public:
    static std::shared_ptr<CBLConfiguration> create(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::chrono::seconds codePairRequestTimeout,
        std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
        std::weak_ptr<aace::engine::alexa::LocaleAssetsManager> localeAssetManager);

    /// @name CBLConfigurationInterface
    /// @{
    std::string getClientId() const override;
    std::string getProductId() const override;
    std::string getDeviceSerialNumber() const override;
    std::chrono::seconds getRequestTimeout() const override;
    std::chrono::seconds getCodePairRequestTimeout() const override;
    std::chrono::seconds getAccessTokenRefreshHeadStart() const override;
    std::string getRequestCodePairUrl() const override;
    std::string getRequestTokenUrl() const override;
    std::string getRefreshTokenUrl() const override;
    std::string getScopeData() const override;
    std::string getDefaultLocale() const override;
    /// @}

private:
    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::chrono::seconds codePairRequestTimeout,
        std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
        std::weak_ptr<aace::engine::alexa::LocaleAssetsManager> localeAssetManager);

    bool initScopeData();

    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
    std::chrono::seconds m_requestTimeout;
    std::chrono::seconds m_codePairRequestTimeout;
    std::chrono::seconds m_accessTokenRefreshHeadStart;
    std::string m_scopeData;
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> m_alexaEndpoints;

    /// Weak reference to the @c LocaleAssetsManager for getting the default locale.
    std::weak_ptr<aace::engine::alexa::LocaleAssetsManager> m_localeAssetManager;
};

}  // namespace cbl
}  // namespace engine
}  // namespace aace

#endif