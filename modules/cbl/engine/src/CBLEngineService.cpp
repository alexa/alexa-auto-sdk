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

#include <typeinfo>
#include <climits>
#include <iostream>

#include <AACE/Alexa/AlexaProperties.h>
#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Authorization/AuthorizationServiceInterface.h"
#include "AACE/Engine/CBL/CBLEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AACE/Engine/PropertyManager/PropertyManagerEngineService.h>
#include <AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h>
#include "AACE/Engine/Utils/JSON/JSON.h"

#include "AACE/Engine/CBL/CBLAuthorizationProvider.h"

namespace aace {
namespace engine {
namespace cbl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.cbl.CBLEngineService");

// Default for code pair request timeout
static const std::chrono::minutes DEFAULT_REQUEST_TIMEOUT = std::chrono::minutes(1);

/// Service name used for @c Authorization and @c AuthorizationManager
static const std::string SERVICE_NAME = "alexa:cbl";

// register the service
REGISTER_SERVICE(CBLEngineService);

CBLEngineService::CBLEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description),
        m_codePairRequestTimeout(DEFAULT_REQUEST_TIMEOUT),
        m_enableUserProfile(false) {
}

bool CBLEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        auto document = aace::engine::utils::json::parse(configuration);
        ThrowIfNull(document, "parseConfigurationStreamFailed");

        auto cblConfigRoot = document->GetObject();

        if (cblConfigRoot.HasMember("requestTimeout") && cblConfigRoot["requestTimeout"].IsUint()) {
            m_codePairRequestTimeout = std::chrono::seconds(cblConfigRoot["requestTimeout"].GetUint());
        }

        if (cblConfigRoot.HasMember("enableUserProfile") && cblConfigRoot["enableUserProfile"].IsBool()) {
            m_enableUserProfile = cblConfigRoot["enableUserProfile"].GetBool();
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}

bool CBLEngineService::setup() {
    try {
        auto alexaComponents =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "invalidAlexaComponentInterface");

        auto deviceInfo = alexaComponents->getDeviceInfo();
        ThrowIfNull(deviceInfo, "invalidDeviceInfo");

        auto alexaEndpoints =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaEndpointInterface>("aace.alexa");

        auto localeAssetManager =
            getContext()->getServiceInterface<aace::engine::alexa::LocaleAssetsManager>("aace.alexa");
        ThrowIfNull(localeAssetManager, "invalidLocaleAssetManager");

        std::shared_ptr<CBLConfiguration> configuration =
            CBLConfiguration::create(deviceInfo, m_codePairRequestTimeout, alexaEndpoints, localeAssetManager);
        ThrowIfNull(configuration, "nullCBLAuthDelegateConfiguration");

        auto authorizationService =
            getContext()->getServiceInterface<aace::engine::authorization::AuthorizationServiceInterface>(
                "aace.authorization");
        if (authorizationService) {
            auto propertyManager =
                getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                    "aace.propertyManager");
            ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

            m_cblAuthorizationProvider = CBLAuthorizationProvider::create(
                SERVICE_NAME,
                alexaComponents->getAuthorizationManager(),
                configuration,
                propertyManager,
                m_enableUserProfile);
            authorizationService->registerProvider(m_cblAuthorizationProvider, SERVICE_NAME);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "setup").d("reason", ex.what()));
        return false;
    }
}

bool CBLEngineService::start() {
    if (m_cblEngineImpl != nullptr) {
        m_cblEngineImpl->enable();
    }
    return true;
}

bool CBLEngineService::stop() {
    if (m_cblEngineImpl != nullptr) {
        m_cblEngineImpl->disable();
    }
    return true;
}

bool CBLEngineService::shutdown() {
    if (m_cblAuthorizationProvider != nullptr) {
        m_cblAuthorizationProvider->shutdown();
        m_cblAuthorizationProvider.reset();
    }

    if (m_cblEngineImpl != nullptr) {
        m_cblEngineImpl->shutdown();
        m_cblEngineImpl.reset();
    }
    return true;
}

bool CBLEngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        return registerPlatformInterfaceType<aace::cbl::CBL>(platformInterface);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool CBLEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::cbl::CBL> cbl) {
    try {
        ThrowIfNotNull(m_cblEngineImpl, "platformInterfaceAlreadyRegistered");

        auto alexaComponents =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "invalidAlexaComponentInterface");

        auto deviceInfo = alexaComponents->getDeviceInfo();
        ThrowIfNull(deviceInfo, "invalidDeviceInfo");

        auto alexaEndpoints =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaEndpointInterface>("aace.alexa");

        auto localeAssetManager =
            getContext()->getServiceInterface<aace::engine::alexa::LocaleAssetsManager>("aace.alexa");
        ThrowIfNull(localeAssetManager, "invalidLocaleAssetManager");

        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

        m_cblEngineImpl = aace::engine::cbl::CBLEngineImpl::create(
            cbl,
            alexaComponents->getAuthorizationManager(),
            deviceInfo,
            m_codePairRequestTimeout,
            alexaEndpoints,
            localeAssetManager,
            propertyManager,
            m_enableUserProfile);
        ThrowIfNull(m_cblEngineImpl, "createCBLEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<CBL>").d("reason", ex.what()));
        return false;
    }
}

}  // namespace cbl
}  // namespace engine
}  // namespace aace
