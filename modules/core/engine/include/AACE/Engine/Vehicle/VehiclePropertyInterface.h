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

#ifndef AACE_ENGINE_VEHICLE_INTERFACE_VEHICLE_PROPERTY_INTERFACE_H
#define AACE_ENGINE_VEHICLE_INTERFACE_VEHICLE_PROPERTY_INTERFACE_H

#include <unordered_map>

#include "AACE/Vehicle/VehicleConfiguration.h"

namespace aace {
namespace engine {
namespace vehicle {

/// Used to specialize std::hash for an @c std::unordered_map with @c VehiclePropertyType key
struct EnumHash {
    template <typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

/// Aliases for readability
/// @{
using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;
using VehiclePropertyMap = std::unordered_map<VehiclePropertyType, std::string, EnumHash>;
/// @}

/**
 * VehiclePropertyInterface allows components to retrieve vehicle properties from the "aace.vehicle" configuration
 */
class VehiclePropertyInterface {
public:
    /// Destructor
    virtual ~VehiclePropertyInterface();

    /**
     * Retrieves the property setting in "aace.vehicle" configuration corresponding to the provided 
     * @c VehiclePropertyType key. The value returned will be an empty string if the value was not configured.
     *
     * @param type The property corresponding to the value to retrieve
     * @return The "aace.vehicle" value corresponding to the provided property type or an empty string if not present.
     */
    virtual std::string getVehicleProperty(VehiclePropertyType type) = 0;

    /**
     * Retrieves a map of @c VehiclePropertyType keys to string values from "aace.vehicle" configuration. Only
     * properties present in "aace.vehicle" configuration will be in the map.
     *
     * @note The map will be empty if "aace.vehicle" does not have any properties configured.
     * 
     * @return The map of @c VehiclePropertyType keys to their string value settings.
     */
    virtual VehiclePropertyMap getVehicleProperties() = 0;

    /**
     * Queries whether "aace.vehicle.info" was configured.
     *
     * @return @c true if configured, @c false otherwise
     */
    virtual bool isVehicleInfoConfigured() = 0;
};

}  // namespace vehicle
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_VEHICLE_INTERFACE_VEHICLE_PROPERTY_INTERFACE_H
