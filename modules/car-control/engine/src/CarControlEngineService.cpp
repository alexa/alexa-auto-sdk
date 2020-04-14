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

#include <string>
#include <typeinfo>

#include "AACE/Alexa/AlexaProperties.h"
#include "AACE/Engine/Alexa/AlexaComponentInterface.h"
#include "AACE/Engine/CarControl/CarControlEngineService.h"
#include "AACE/Engine/CarControl/Endpoint.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Storage/StorageEngineService.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace aace {
namespace engine {
namespace carControl {

/// String to identify log entries originating from this file
static const std::string TAG("aace.carControl.CarControlEngineService");

/// The key for the AACE Alexa service interface
static const std::string AACE_ALEXA_SERVICE_KEY = "aace.alexa";
/// The key for the AACE storage service interface
static const std::string AACE_STORAGE_SERVICE_KEY = "aace.storage";

/// The name of the 'carControl' table in the database
static const std::string CAR_CONTROL_CONFIG_TABLE = "carControl";
/// The key for the 'configutation' in the database 'carControl' table
static const std::string CAR_CONTROL_CONFIG_KEY = "configuration";

/// The key for the 'endpoints' node of configuration
static const std::string CONFIG_KEY_ENDPOINTS = "endpoints";
/// The key for the 'assets' node of configuration
static const std::string CONFIG_KEY_ASSETS = "assets";
/// The key for the 'defaultAssetsPath' node of configuration
static const std::string CONFIG_KEY_DEFAULT_ASSETS_PATH = "defaultAssetsPath";
/// The key for the 'customAssetsPath' node of configuration
static const std::string CONFIG_KEY_CUSTOM_ASSETS_PATH = "customAssetsPath";

// Register the car control service with the Engine
REGISTER_SERVICE(CarControlEngineService);

CarControlEngineService::CarControlEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

CarControlEngineService::~CarControlEngineService() = default;

bool CarControlEngineService::initialize() {
    try {
        ThrowIfNot(
            registerServiceInterface<CarControlEngineService>(shared_from_this()),
            "registerCarControlEngineServiceFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool CarControlEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        AACE_DEBUG(LX(TAG).d("isLocalServiceAvailable", isLocalServiceAvailable()));
        ThrowIf(m_configured, "carControlEngineServiceAlreadyConfigured");

        json jconfiguration;
        jconfiguration = json::parse(*configuration);

        // Ingest assets from the file path(s) specified in configuration, else use the hardcoded default assets, and
        // store them in an @c AssetStore. This facilitates retrieval of friendly name/locale pairs for an Endpoint
        // during its construction.
        if (jconfiguration.contains(CONFIG_KEY_ASSETS) && jconfiguration[CONFIG_KEY_ASSETS].is_object()) {
            auto& assets = jconfiguration.at(CONFIG_KEY_ASSETS);
            if (assets.contains(CONFIG_KEY_DEFAULT_ASSETS_PATH) && assets[CONFIG_KEY_DEFAULT_ASSETS_PATH].is_string()) {
                std::string path = assets.at(CONFIG_KEY_DEFAULT_ASSETS_PATH);
                AACE_DEBUG(LX(TAG).m("addingDefaultAssetsFromPath").sensitive("path", path));
                ThrowIfNot(m_assetStore.addAssets(path), "addDefaultAssetsFromPathFailed");
            } else {
                ThrowIfNot(m_assetStore.addDefaultAssets(), "addDefaultAssetsFailed");
            }
            if (assets.contains(CONFIG_KEY_CUSTOM_ASSETS_PATH) && assets[CONFIG_KEY_CUSTOM_ASSETS_PATH].is_string()) {
                std::string path = assets.at(CONFIG_KEY_CUSTOM_ASSETS_PATH);
                AACE_DEBUG(LX(TAG).m("addingCustomAssetsFromPath").sensitive("path", path));
                ThrowIfNot(m_assetStore.addAssets(path), "addCustomAssetsFromPathFailed");
            }
        } else {
            ThrowIfNot(m_assetStore.addDefaultAssets(), "addDefaultAssetsFailed");
        }

        // Construct an object representation of each endpoint in configuration
        if (jconfiguration.contains(CONFIG_KEY_ENDPOINTS) && jconfiguration.at(CONFIG_KEY_ENDPOINTS).is_array()) {
            for (auto& item : jconfiguration.at(CONFIG_KEY_ENDPOINTS).items()) {
                auto endpoint = Endpoint::create(item.value(), m_assetStore);
                ThrowIfNull(endpoint, "createEndpointFailed");
                ThrowIfNot(m_endpoints.insert({endpoint->getId(), endpoint}).second, "insertEndpointFailed");
            }
        }

        // Write the configuration to storage for retrieval by the car control local service
        auto localStorage =
            getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>(AACE_STORAGE_SERVICE_KEY);
        ThrowIfNull(localStorage, "invalidLocalStorage");
        std::string s = jconfiguration.dump();
        localStorage->put(CAR_CONTROL_CONFIG_TABLE, CAR_CONTROL_CONFIG_KEY, s);

        m_configured = true;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool CarControlEngineService::setup() {
    try {
        if (m_carControlEngineImpl != nullptr && !isLocalServiceAvailable()) {
            auto alexaComponents =
                getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>(AACE_ALEXA_SERVICE_KEY);
            ThrowIfNull(alexaComponents, "nullAlexaComponentInterface");

            // Endpoint builder factory facilitates creation of an EndpointBuilder for each car control endpoint
            auto endpointBuilderFactory = alexaComponents->getEndpointBuilderFactory();
            ThrowIfNull(endpointBuilderFactory, "nullEndpointBuilderFactory");

            // Car control endpoints use the same "manufacturerName" and "description" as the configured DeviceInfo for
            // the default endpoint since "aace.carControl" config currently does not expect these values to be provided
            std::string manufacturerName = "";
            std::string description = "";
            auto deviceInfo = alexaComponents->getDeviceInfo();
            if (deviceInfo != nullptr) {
                manufacturerName = deviceInfo->getManufacturerName();
                description = deviceInfo->getDeviceDescription();
            }

            for (auto& endpoint : m_endpoints) {
                endpoint.second->build(
                    m_carControlEngineImpl, endpointBuilderFactory, m_assetStore, manufacturerName, description);
            }

            // The contents of the AssetStore won't be used again, so we can release the memory
            m_assetStore.clear();
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_assetStore.clear();
        return false;
    }
}

bool CarControlEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::carControl::CarControl>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool CarControlEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::carControl::CarControl> platformInterface) {
    try {
        ThrowIfNotNull( m_carControlEngineImpl, "platformInterfaceAlreadyRegistered" );

        m_carControlEngineImpl = CarControlEngineImpl::create(platformInterface);
        ThrowIfNull(m_carControlEngineImpl, "createCarControlEngineImplFailed");

        ThrowIfNot(
            registerServiceInterface<CarControlServiceInterface>(m_carControlEngineImpl),
            "registerCarControlServiceInterfaceFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<aace::carControl::CarControl>").d("reason", ex.what()));
        return false;
    }
}

void CarControlEngineService::setLocalServiceAvailability(bool isAvailable) {
    m_isLocalServiceAvailable = isAvailable;
}

bool CarControlEngineService::isLocalServiceAvailable() {
    return m_isLocalServiceAvailable;
}

bool CarControlEngineService::shutdown() {
    AACE_INFO(LX(TAG));

    if (m_carControlEngineImpl != nullptr) {
        m_carControlEngineImpl->shutdown();
        m_carControlEngineImpl.reset();
    }

    return true;
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
