/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <Endpoints/EndpointBuilder.h>

#include <AACE/Engine/CarControl/AssetStore.h>
#include <AACE/Engine/CarControl/CapabilityController.h>
#include <AACE/Engine/CarControl/Endpoint.h>
#include <AACE/Engine/CarControl/ModeController.h>
#include <AACE/Engine/CarControl/PowerController.h>
#include <AACE/Engine/CarControl/RangeController.h>
#include <AACE/Engine/CarControl/ToggleController.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace carControl {

/// String to identify log entries originating from this file
static const std::string TAG("aace.carControl.Endpoint");

/// The namespace and name of the ModeController capability interface
static const std::string CAPABILITY_MODE_CONTROLLER = "Alexa.ModeController";
/// The namespace and name of the PowerController capability interface
static const std::string CAPABILITY_POWER_CONTROLLER = "Alexa.PowerController";
/// The namespace and name of the RangeController capability interface
static const std::string CAPABILITY_RANGE_CONTROLLER = "Alexa.RangeController";
/// The namespace and name of the ToggleController capability interface
static const std::string CAPABILITY_TOGGLE_CONTROLLER = "Alexa.ToggleController";

/// The 'assetId' configuration key
static const std::string CONFIG_KEY_ASSET_ID = "assetId";
/// The 'capabilities' configuration key
static const std::string CONFIG_KEY_CAPABILITIES = "capabilities";
/// The 'endpointId' configuration key
static const std::string CONFIG_KEY_ENDPOINT_ID = "endpointId";
/// The 'endpointResources' configuration key
static const std::string CONFIG_KEY_ENDPOINT_RESOURCES = "endpointResources";
/// The 'friendlyNames' configuration key
static const std::string CONFIG_KEY_FRIENDLY_NAMES = "friendlyNames";
/// The 'interface' configuration key
static const std::string CONFIG_KEY_INTERFACE = "interface";
/// The 'type' configuration key
static const std::string CONFIG_KEY_TYPE = "@type";
/// The 'value' configuration key
static const std::string CONFIG_KEY_VALUE = "value";

/// The display category for the endpoint in the companion app
static const std::string DISPLAY_CATEGORY = "VEHICLE";

std::shared_ptr<Endpoint> Endpoint::create(const json& endpointConfig, AssetStore& assetStore) {
    try {
        ThrowIfNot(endpointConfig.contains(CONFIG_KEY_ENDPOINT_ID), "noEndpointId");
        std::string endpointId = endpointConfig.at(CONFIG_KEY_ENDPOINT_ID);
        ThrowIfNot(!endpointId.empty(), "emptyEndpointId");

        // Get all asset IDs used to identify this endpoint
        ThrowIfNot(endpointConfig.contains(CONFIG_KEY_ENDPOINT_RESOURCES), "noEndpointResources");
        auto& endpointResources = endpointConfig.at(CONFIG_KEY_ENDPOINT_RESOURCES);
        ThrowIfNot(endpointResources.contains(CONFIG_KEY_FRIENDLY_NAMES), "noFriendlyNames");
        auto friendlyNames = endpointResources.at(CONFIG_KEY_FRIENDLY_NAMES);
        std::vector<std::string> assetIds;
        for (auto& item : friendlyNames.items()) {
            auto& type = item.value().at(CONFIG_KEY_TYPE);
            ThrowIfNot(type == "asset", "expectedAssetTypeResource");
            auto& value = item.value().at(CONFIG_KEY_VALUE);
            std::string assetId = value.at(CONFIG_KEY_ASSET_ID);
            assetIds.push_back(assetId);
        }

        // Retrieve all friendly names for this endpoint from the @c AssetStore
        std::vector<std::string> names;
        for (auto assetId = assetIds.begin(); assetId != assetIds.end(); ++assetId) {
            const std::vector<std::string>& friendlyNames = assetStore.getValues(*assetId);
            names.insert(names.end(), friendlyNames.begin(), friendlyNames.end());
        }

        auto endpoint = std::shared_ptr<Endpoint>(new Endpoint(endpointId, names));
        ThrowIfNull(endpoint, "cannotCreateEndpoint");

        // Add capability controllers to the Endpoint
        ThrowIfNot(endpointConfig.contains(CONFIG_KEY_CAPABILITIES), "noCapabilities");
        auto& capabilities = endpointConfig.at(CONFIG_KEY_CAPABILITIES);
        for (auto& item : capabilities.items()) {
            auto& capabilityConfig = item.value();
            std::string interface = capabilityConfig.at(CONFIG_KEY_INTERFACE);
            std::shared_ptr<CapabilityController> controller = nullptr;
            if (interface == CAPABILITY_POWER_CONTROLLER) {
                controller = PowerController::create(endpointId, interface);
                ThrowIfNull(controller, "createPowerControllerFailed");
            } else if (interface == CAPABILITY_TOGGLE_CONTROLLER) {
                controller = ToggleController::create(capabilityConfig, endpointId, interface, assetStore);
                ThrowIfNull(controller, "createToggleControllerFailed");
            } else if (interface == CAPABILITY_RANGE_CONTROLLER) {
                controller = RangeController::create(capabilityConfig, endpointId, interface, assetStore);
                ThrowIfNull(controller, "createToggleControllerFailed");
            } else if (interface == CAPABILITY_MODE_CONTROLLER) {
                controller = ModeController::create(capabilityConfig, endpointId, interface, assetStore);
                ThrowIfNull(controller, "createModeControllerFailed");
            } else {
                Throw("unsupportedCapability");
            }

            if (controller) {
                ThrowIfNot(endpoint->addController(controller->getId(), controller), "addControllerFailed");
            }
        }
        return endpoint;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool Endpoint::addController(const std::string& id, std::shared_ptr<CapabilityController> controller) {
    auto result = m_controllers.emplace(controller->getId(), controller);
    return result.second;
}

Endpoint::Endpoint(const std::string endpointId, const std::vector<std::string>& names) :
        m_endpointId{endpointId},
        m_names{names} {
}

Endpoint::~Endpoint() {
    m_assetIds.clear();
    m_names.clear();
    m_controllers.clear();
}

std::string Endpoint::getId() {
    return m_endpointId;
}

bool Endpoint::build(
    std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
    std::shared_ptr<aace::engine::alexa::EndpointBuilderFactory> endpointBuilderFactory,
    const std::string& manufacturer) {
    try {
        auto id = getId();
        std::string derivedId;
        // For every friendly name used to describe this endpoint, create a uniquely-identified duplicate AVS SDK
        // Endpoint representation since 'endpointResources' is not supported in the cloud
        int index = 0;
        for (auto name = m_names.begin(); name != m_names.end(); ++name, ++index) {
            auto endpointBuilder = endpointBuilderFactory->createEndpointBuilder();
            ThrowIfNull(endpointBuilder, "couldNotCreateEndpointBuilder");
            // For each duplicate endpoint:
            // 1. Create a unique endpoint ID by appending the index to the base endpoint ID
            // 2. Associate the duplicate endpoint with the base endpoint through the 'isEquivalentOf' cookie
            if (index) {
                endpointBuilder->withDerivedEndpointId(id + "." + std::to_string(index));
                endpointBuilder->withCookies({{"isEquivalentOf", derivedId}});
            } else {
                endpointBuilder->withDerivedEndpointId(id);
            }
            endpointBuilder->withFriendlyName(*name);
            endpointBuilder->withDisplayCategory({DISPLAY_CATEGORY});
            endpointBuilder->withDescription({*name});
            endpointBuilder->withManufacturerName({manufacturer});

            for (auto& item : m_controllers) {
                auto controller = item.second;
                controller->build(carControlServiceInterface, endpointBuilder);
            }
            auto endpointId = endpointBuilder->build();
            ThrowIfNot(endpointId.hasValue(), "couldNotBuildEndpoint");
            if (derivedId.empty()) {
                derivedId = endpointId.value();
            }
            endpointBuilder.reset();
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
