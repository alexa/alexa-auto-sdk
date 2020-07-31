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

#ifndef AACE_ENGINE_CAR_CONTROL_ZONE_DEFINITIONS_H
#define AACE_ENGINE_CAR_CONTROL_ZONE_DEFINITIONS_H

#include <nlohmann/json.hpp>
#include <unordered_map>

#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>

#include "AACE/Engine/CarControl/AssetStore.h"

namespace aace {
namespace engine {
namespace carControl {

/**
 * @c ZoneDefinitions is responsible for constructing 
 * Alexa.Automotive.ZoneDefinitions capability configuration.
 * 
 * An object of this class is constructed with the JSON already in valid 
 * capability configuration format, but the cloud requires the following updates:
 * 1. Translate asset IDs to text because the cloud still cannot accept a 
 *    custom assets catalog.
 * 2. Translate the configured endpoint IDs in a zone's 'members' array to 
 *    the derived endpoint IDs that are actually sent to the cloud in discovery.
 *
 * This class does the above translation and provides the capability 
 * configuration to AVS Device SDK for discovery event construction.
 */
class ZoneDefinitions : public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface {
public:
    /// Aliases for readability
    /// @{
    using CapabilityConfiguration = alexaClientSDK::avsCommon::avs::CapabilityConfiguration;
    using json = nlohmann::json;
    /// @}

    /**
     * Create a ZoneDefinitions capability provider.
     *
     * @param jconfig The capability configuration as JSON.
     * @param assetStore The @c AssetStore used to translate assets to string literals.
     * @param endpointIdMappings A list of mappings of configured endpoint IDs to the IDs sent in discovery.
     * @return A shared pointer to a @c ZoneDefinitions, or @c nullptr if construction fails due to invalid arguments
     */
    static std::shared_ptr<ZoneDefinitions> create(
        const json& jconfig,
        const AssetStore& assetStore,
        const std::unordered_map<std::string, std::string>& endpointIdMappings);

    /// @name CapabilityConfigurationInterface methods
    /// @{
    std::unordered_set<std::shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
    /// @}

private:
    /**
     * ZoneDefinitions constructor.
     *
     * @param capabilityConfig A valid @c CapabilityConfiguration for the Alexa.Automotive.ZoneDefinitions configuration
     *        provided by this object.
     */
    ZoneDefinitions(std::shared_ptr<CapabilityConfiguration> capabilityConfig);

    /// Set of capability configurations that will be published in the discovery message
    std::unordered_set<std::shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_ZONE_DEFINITIONS_H
