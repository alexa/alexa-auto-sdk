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

#ifndef AACE_ENGINE_ALEXA_VEHICLEDATA_H
#define AACE_ENGINE_ALEXA_VEHICLEDATA_H

#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/Utils/Optional.h>

#include "AACE/Engine/Vehicle/VehiclePropertyInterface.h"

#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace alexa {

/**
 * VehicleData is responsible for constructing Alexa.Automotive.VehicleData capability configuration
 */
class VehicleData : public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface {
public:
    /// Aliases for readability
    /// @{
    using VehiclePropertyMap = aace::engine::vehicle::VehiclePropertyMap;
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;
    using CapabilityConfiguration = alexaClientSDK::avsCommon::avs::CapabilityConfiguration;
    /// @}

    /**
     * Create a VehicleData capability provider.
     *
     * @param vehiclePropertyMap A map of @c VehiclePropertyType keys to the values set in "aace.vehicle" configuration
     * @return A shared pointer to a @c VehicleData, or @c nullptr if construction fails due to invalid arguments
     */
    static std::shared_ptr<VehicleData> create(const VehiclePropertyMap& vehiclePropertyMap);

    /// @name CapabilityConfigurationInterface methods
    /// @{
    std::unordered_set<std::shared_ptr<CapabilityConfiguration>> getCapabilityConfigurations() override;
    /// @}

private:
    /**
     * VehicleData constructor.
     *
     * @param capabilityConfig A valid @c CapabilityConfiguration for the Alexa.Automotive.VehicleData configuration
     *        provided by this object.
     */
    VehicleData(std::shared_ptr<CapabilityConfiguration> capabilityConfig);

    /**
     * Creates the Alexa.Automotive.VehicleData capability configuration.
     *
     * @return The Alexa.Automotive.VehicleData @c CapabilityConfiguration or @c nullptr if constucting the
     * configuration fails
     */
    static std::shared_ptr<CapabilityConfiguration> getVehicleDataCapabilityConfiguration(
        const VehiclePropertyMap& vehiclePropertyMap);

    /**
     * Retrieves the setting from @c vehiclePropertyMap for the @c VehiclePropertyType represented by the
     * provided string. For example, capability attribute "MICROPHONE_TYPE" returns the OEM setting for the
     * VehiclePropertyType::MICROPHONE property.
     *
     * @param attribute The key of an attribute (analytics, resources, or additional) used in
     *        Alexa.Automotive.VehicleData capability configuration
     * @return An optional value containing the OEM setting from @c VehicleConfiguration corresponding to the provided 
     *        key if present, else an uninitialized value
     */
    static alexaClientSDK::avsCommon::utils::Optional<std::string> getPropertyByAttribute(
        const std::string& attribute,
        const VehiclePropertyMap& vehiclePropertyMap);

    /**
     * Utility function to generate a rapidjson::Value representation of an analytics data object for capability
     * configuration.
     *
     * For example,
     * @code{.json}
     * {
     *  "key": "OPERATING_SYSTEM",
     *  "value": "AndroidOreo_8.1"
     * }
     * @endcode
     *
     * @param key The key for the object
     * @param value The literal string representation of the value
     * @param allocator The rapidJSON allocator
     * @return A @c rapidjson::Value representing the JSON
     */
    static rapidjson::Value getAnalyticsObject(
        const std::string& key,
        const std::string& value,
        rapidjson::Document::AllocatorType& allocator);

    /**
     * Utility function to generate a rapidjson::Value representation of a text Label using the provided string value.
     * This is to populate the resources of the capability configuration.
     * See https://developer.amazon.com/en-US/docs/alexa/device-apis/resources-and-assets.html#label-object
     *
     * For example,
     * @code{.json}
     * {
     *  "@type": "text",
     *  "value": {
     *      "text" : "vehicle_make_name",
     *      "locale" : "en-US"
     *  }
     * }
     * @endcode
     *
     * @param value The literal string representation of the value
     * @param allocator The rapidJSON allocator
     * @return A @c rapidjson::Value representing the JSON
     */
    static rapidjson::Value getTextResourceObject(
        const std::string& value,
        rapidjson::Document::AllocatorType& allocator);

    /// Set of capability configurations that will be published in the discovery message
    std::unordered_set<std::shared_ptr<CapabilityConfiguration>> m_capabilityConfigurations;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_VEHICLEDATA_H
