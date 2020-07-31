/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/CarControl/AssetStore.h>
#include <AACE/Engine/CarControl/CapabilityController.h>
#include <AACE/Engine/CarControl/Endpoint.h>
#include <AACE/Engine/CarControl/ModeController.h>
#include <AACE/Engine/CarControl/PowerController.h>
#include <AACE/Engine/CarControl/RangeController.h>
#include <AACE/Engine/CarControl/ToggleController.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AVSCommon/AVS/EndpointResources.h>
#include <Endpoints/EndpointBuilder.h>

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

/// The 'endpointId' configuration key
static const std::string CONFIG_KEY_ENDPOINT_ID = "endpointId";
/// The 'endpointResources' configuration key
static const std::string CONFIG_KEY_ENDPOINT_RESOURCES = "endpointResources";
/// The 'friendlyNames' configuration key
static const std::string CONFIG_KEY_FRIENDLY_NAMES = "friendlyNames";
/// The 'type' configuration key
static const std::string CONFIG_KEY_TYPE = "@type";
/// The 'asset' label type
static const std::string CONFIG_TYPE_ASSET = "asset";
/// The 'value' configuration key
static const std::string CONFIG_KEY_VALUE = "value";
/// The 'assetId' configuration key
static const std::string CONFIG_KEY_ASSET_ID = "assetId";

/// The 'capabilities' configuration key
static const std::string CONFIG_KEY_CAPABILITIES = "capabilities";
/// The 'interface' configuration key
static const std::string CONFIG_KEY_INTERFACE = "interface";

/// The display category for the endpoint in the companion app
static const std::string DISPLAY_CATEGORY = "VEHICLE";

std::shared_ptr<Endpoint> Endpoint::create(const json& endpointConfig, const AssetStore& assetStore) {
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
            // For now, "text" labels
            // (https://developer.amazon.com/en-US/docs/alexa/device-apis/resources-and-assets.html#label-object) are
            // not allowed, since aace.carControl config has always expected assets
            ThrowIfNot(type == CONFIG_TYPE_ASSET, "expectedAssetTypeResource");
            auto& value = item.value().at(CONFIG_KEY_VALUE);
            std::string assetId = value.at(CONFIG_KEY_ASSET_ID);
            assetIds.push_back(assetId);
        }

        auto endpoint = std::shared_ptr<Endpoint>(new Endpoint(endpointId, assetIds));
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
                ThrowIfNull(controller, "createRangeControllerFailed");
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

Endpoint::Endpoint(const std::string endpointId, const std::vector<std::string>& assetIds) :
        m_endpointId{endpointId}, m_assetIds{assetIds} {
}

Endpoint::~Endpoint() {
    m_assetIds.clear();
    m_controllers.clear();
}

std::string Endpoint::getId() {
    return m_endpointId;
}

std::string Endpoint::getDiscoveryId() {
    return m_discoveryEndpointId;
}

bool Endpoint::build(
    std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
    std::shared_ptr<aace::engine::alexa::EndpointBuilderFactory> endpointBuilderFactory,
    const AssetStore& assetStore,
    const std::string& manufacturer,
    const std::string& description) {
    try {
        auto endpointBuilder = endpointBuilderFactory->createEndpointBuilder();
        ThrowIfNull(endpointBuilder, "couldNotCreateEndpointBuilder");

        alexaClientSDK::avsCommon::avs::EndpointResources endpointResources;
        for (auto asset = m_assetIds.begin(); asset != m_assetIds.end(); ++asset) {
            const std::vector<AssetStore::NameLocalePair>& names = assetStore.getFriendlyNames(*asset);
            for (auto name = names.begin(); name != names.end(); ++name) {
                endpointResources.addFriendlyNameWithText(name->first, name->second);
            }
        }
        // Note: "aace.carControl" config currently does not use manufacturer name, so we fill in a default
        endpointResources.addManufacturerNameWithText(manufacturer, "en-US");
        // Note: "aace.carControl" config currently does not use a description, so we fill in a default
        endpointResources.addDescriptionWithText(description, "en-US");
        endpointBuilder->withEndpointResources(endpointResources);
        endpointBuilder->withDerivedEndpointId(getId());
        endpointBuilder->withDisplayCategory({DISPLAY_CATEGORY});

        for (auto& item : m_controllers) {
            auto controller = item.second;
            controller->build(carControlServiceInterface, endpointBuilder);
        }
        auto endpointId = endpointBuilder->build();
        ThrowIfNot(endpointId.hasValue(), "couldNotBuildEndpoint");
        m_discoveryEndpointId = endpointId.value();
        endpointBuilder.reset();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
