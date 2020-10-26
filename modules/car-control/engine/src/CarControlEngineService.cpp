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

#include "AACE/Engine/CarControl/CarControlEngineService.h"

#include <string>
#include <typeinfo>
#include <unordered_map>

#include "AACE/Alexa/AlexaProperties.h"
#include "AACE/Engine/Alexa/AlexaComponentInterface.h"
#include "AACE/Engine/CarControl/Endpoint.h"
#include "AACE/Engine/CarControl/ZoneDefinitions.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Storage/StorageEngineService.h"

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

// The endpoint ID of the internal endpoint created for zones
static const std::string INTERNAL_ENDPOINT_ID = "_AutoSDKInternalRoot";

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

        // Translate <v2.2 zones config format (top level "zones" array) to v2.3+ (ZoneDefinitions capability)
        translateConfigForZones(jconfiguration);

        // Construct an object representation of each endpoint in configuration
        if (jconfiguration.contains(CONFIG_KEY_ENDPOINTS) && jconfiguration.at(CONFIG_KEY_ENDPOINTS).is_array()) {
            for (auto& item : jconfiguration.at(CONFIG_KEY_ENDPOINTS).items()) {
                if (item.value().at("endpointId") == INTERNAL_ENDPOINT_ID) continue;
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
        if (m_carControlEngineImpl != nullptr) {
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

            // Get the IDs for each endpoint used in discovery. Used for translation for ZoneDefinitions
            std::unordered_map<std::string, std::string> endpointIdMappings;

            for (auto& endpoint : m_endpoints) {
                endpoint.second->build(
                    m_carControlEngineImpl, endpointBuilderFactory, m_assetStore, manufacturerName, description);
                endpointIdMappings.insert({endpoint.second->getId(), endpoint.second->getDiscoveryId()});
            }

            // Add ZoneDefinitions capability to a dummy endpoint
            // Note: Only cloud discovery code path uses this.
            // A dummy endpoint was added to the LVC config already in translateConfigForZones()
            if (m_zonesCapabilityConfig.contains("configuration")) {
                auto zoneDefinitions = aace::engine::carControl::ZoneDefinitions::create(
                    m_zonesCapabilityConfig, m_assetStore, endpointIdMappings);
                ThrowIfNull(zoneDefinitions, "couldNotConstructZoneDefinitions");
                auto endpointBuilder = endpointBuilderFactory->createEndpointBuilder();
                endpointBuilder->withCapabilityConfiguration(zoneDefinitions);
                endpointBuilder->withDerivedEndpointId(INTERNAL_ENDPOINT_ID);
                endpointBuilder->withFriendlyName(INTERNAL_ENDPOINT_ID);
                endpointBuilder->withDescription("Internal reference endpoint");
                endpointBuilder->withManufacturerName(deviceInfo->getManufacturerName());
                endpointBuilder->withDisplayCategory({"VEHICLE"});
                endpointBuilder->withCookies({{"createdBy", "AutoSDK"}});
                auto endpointId = endpointBuilder->build();
                ThrowIfNot(endpointId.hasValue(), "couldNotBuildInternalReferenceEndpoint");
                endpointBuilder.reset();
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
        ThrowIfNotNull(m_carControlEngineImpl, "platformInterfaceAlreadyRegistered");

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

void CarControlEngineService::translateConfigForZones(json& jconfiguration) {
    try {
        if (jconfiguration.contains("zones") && jconfiguration.at("zones").is_array()) {
            // Construct a 'ZoneDefinitions' capability and populate its "zones" array of
            // capability configuration with the top-level "zones" array of "aace.carControl"
            // clang-format off
            json capability = {
                {"type", "AlexaInterface"},
                {"interface", "Alexa.Automotive.ZoneDefinitions"},
                {"version", "1.0"},
                {"configuration", {
                    {"zones", jconfiguration["zones"]}
                }}
            };
            // clang-format on
            // Remove 'zones' from the top level of configuration
            jconfiguration.erase("zones");

            // Copy default zone ID from top-level "aace.carControl" to ZoneDefinitions capability config
            if (jconfiguration.contains("defaultZoneId")) {
                capability["configuration"].push_back({"defaultZoneId", jconfiguration["defaultZoneId"]});
                jconfiguration.erase("defaultZoneId");
            }

            // Cache the capability configuration for further translation for the cloud discovery (see ZoneDefinitions.h)
            m_zonesCapabilityConfig = capability;

            // Assign the ZoneDefinitions capability to a dummy endpoint
            // Note: Only LVC code path (CarControlLocalService) parses this addition to jconfiguration
            // clang-format off
            json zoneEndpoint = {
                {"endpointId", INTERNAL_ENDPOINT_ID},
                {"friendlyName", INTERNAL_ENDPOINT_ID},
                {"description", "internal reference endpoint"},
                {"cookies", {
                    {"createdBy", "AutoSDK"}
                }},
                {"displayCategories", {"VEHICLE"}},
                {"capabilities", json::array({capability})}
            };
            // clang-format on
            if (!jconfiguration.contains("endpoints")) {
                jconfiguration["endpoints"] = json::array();
            }
            jconfiguration["endpoints"].push_back(zoneEndpoint);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).m("translationFailed").d("reason", ex.what()));
    }
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
