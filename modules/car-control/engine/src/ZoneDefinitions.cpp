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
#include "AACE/Engine/CarControl/ZoneDefinitions.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <string>

namespace aace {
namespace engine {
namespace carControl {

/// String to identify log entries originating from this file
static const std::string TAG("aace.carControl.ZoneDefinitions");

/// ZoneDefinitions interface type
static const std::string INTERFACE_TYPE = "AlexaInterface";
/// ZoneDefinitions interface name
static const std::string INTERFACE_NAME = "Alexa.Automotive.ZoneDefinitions";
/// ZoneDefinitions interface version
static const std::string INTERFACE_VERSION = "1.0";

std::shared_ptr<ZoneDefinitions> ZoneDefinitions::create(
    const json& jconfig,
    const AssetStore& assetStore,
    const std::unordered_map<std::string, std::string>& endpointIdMappings) {
    try {
        ThrowIfNot(
            jconfig.contains("configuration") && jconfig.at("configuration").is_object(), "missingConfigurationObject");
        json configuration = jconfig.at("configuration");
        ThrowIfNot(configuration.contains("zones") && configuration.at("zones").is_array(), "missingZonesArray");

        // clang-format off
        json translated = {
            {"zones", json::array()}
        };
        // clang-format on
        if (configuration.contains("defaultZoneId") && configuration.at("defaultZoneId").is_string()) {
            translated["defaultZoneId"] = configuration.at("defaultZoneId");
        }

        json zonesArray = configuration.at("zones");
        for (auto& zone : zonesArray) {
            ThrowIfNot(zone.contains("zoneId") && zone.at("zoneId").is_string(), "invalidZoneId");
            ThrowIfNot(zone.contains("zoneResources") && zone.at("zoneResources").is_object(), "invalidZoneResources");
            ThrowIfNot(zone.contains("members") && zone.at("members").is_array(), "invalidMembers");
            std::string zoneId = zone.at("zoneId");

            // clang-format off
            json zoneDef = {
                {"zoneId", zoneId},
                {"zoneResources", json()},
                {"members", json::array()}
            };
            // clang-format on

            // Translate assets in zone resources to text
            json zoneResources = zone.at("zoneResources");
            ThrowIfNot(
                zoneResources.contains("friendlyNames") && zoneResources.at("friendlyNames").is_array(),
                "invalidFriendlyNames");
            json friendlyNames = zoneResources.at("friendlyNames");
            json translatedNames = json::array();
            for (auto& friendlyName : friendlyNames) {
                ThrowIfNot(
                    friendlyName.contains("@type") && friendlyName.at("@type").is_string(), "invalidFriendlyNameType");
                std::string type = friendlyName.at("@type");
                if (type == "text") {
                    translatedNames.push_back(friendlyName);
                } else {
                    ThrowIfNot(type == "asset", "invalidFriendlyNameType");
                    std::string assetId = friendlyName["value"]["assetId"];
                    const std::vector<AssetStore::NameLocalePair>& names = assetStore.getFriendlyNames(assetId);
                    for (auto& name : names) {
                        // clang-format off
                        json translatedName = {
                            {"@type", "text"},
                            {"value", {
                                {"text", name.first},
                                {"locale", name.second},
                            }}
                        };
                        // clang-format on
                        translatedNames.push_back(translatedName);
                    }
                }
            }
            zoneResources["friendlyNames"] = translatedNames;
            zoneDef["zoneResources"] = zoneResources;

            // Translate configured endpoint ID to endpoint ID used in discovery event
            json members = zone.at("members");
            for (auto& member : members) {
                std::string endpointId = member.at("endpointId");
                auto iter = endpointIdMappings.find(endpointId);
                ThrowIf(iter == endpointIdMappings.end(), "endpointIdNotPresentInMap");
                zoneDef["members"].push_back({{"endpointId", iter->second}});
            }
            translated["zones"].push_back(zoneDef);
        }

        auto additionalConfigurations =
            alexaClientSDK::avsCommon::avs::CapabilityConfiguration::AdditionalConfigurations();
        additionalConfigurations.insert({"configuration", translated.dump()});
        auto config = std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(
            INTERFACE_TYPE,
            INTERFACE_NAME,
            INTERFACE_VERSION,
            alexaClientSDK::avsCommon::utils::Optional<std::string>(),
            alexaClientSDK::avsCommon::utils::Optional<
                alexaClientSDK::avsCommon::avs::CapabilityConfiguration::Properties>(),
            additionalConfigurations);
        std::shared_ptr<ZoneDefinitions> zoneDefinitions =
            std::shared_ptr<ZoneDefinitions>(new ZoneDefinitions(config));
        return zoneDefinitions;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", ex.what()));
        return nullptr;
    }
}

ZoneDefinitions::ZoneDefinitions(std::shared_ptr<CapabilityConfiguration> capabilityConfig) {
    m_capabilityConfigurations.insert(capabilityConfig);
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> ZoneDefinitions::
    getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
