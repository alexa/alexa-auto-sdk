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

#ifndef AACE_ENGINE_VEHICLE_VEHICLE_ENGINE_SERVICE_H
#define AACE_ENGINE_VEHICLE_VEHICLE_ENGINE_SERVICE_H

#include <unordered_map>

#include "AACE/Engine/Core/EngineService.h"
#include "AACE/Engine/Metrics/MetricEvent.h"
#include "AACE/Engine/Storage/StorageEngineService.h"
#include "AACE/Engine/Vehicle/VehiclePropertyInterface.h"
#include "AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h"
#include "AACE/Engine/PropertyManager/PropertyManagerEngineService.h"
#include "AACE/Vehicle/VehicleConfiguration.h"

#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace vehicle {

class VehicleEngineService
        : public aace::engine::core::EngineService
        , public VehiclePropertyInterface
        , public std::enable_shared_from_this<VehicleEngineService> {
public:
    DESCRIBE(
        "aace.vehicle",
        VERSION("1.0"),
        DEPENDS(aace::engine::storage::StorageEngineService),
        DEPENDS(aace::engine::propertyManager::PropertyManagerEngineService))

public:
    /// Aliases for readability
    /// @{
    using VehiclePropertyMap = aace::engine::vehicle::VehiclePropertyMap;
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;
    /// @}

private:
    VehicleEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~VehicleEngineService() = default;

    using SetPropertyResultCallback = std::function<void(const std::string&, const std::string&, const std::string&)>;

    /// @name VehiclePropertyInterface methods
    /// @{
    std::string getVehicleProperty(VehiclePropertyType type) override;
    VehiclePropertyMap getVehicleProperties() override;
    bool isVehicleInfoConfigured() override;
    /// @}

    /// Emit vehicle metric
    void record(bool full);

    bool setProperty_operatingCountry(
        const std::string& value,
        bool& changed,
        bool& async,
        const SetPropertyResultCallback& callbackFunction);
    std::string getProperty_operatingCountry();

protected:
    /// @name EngineService methods
    /// @{
    bool initialize() override;
    bool setup() override;
    bool configure(std::shared_ptr<std::istream> configuration) override;
    /// @}

    /**
     * Checks the root document for the presence of 'configKey' and updates 'propertyMap' at 'propertyKey' if the value
     * is present. No default values are inserted if the key is not present.
     * 
     * @param [in] root The root document
     * @param [in] configKey The key in the config document 
     * @param [in] propertyKey The key for the map
     * @param [out] propertyMap The map that will be updated if the key is present
     */
    void getVehicleConfigProperty(
        rapidjson::Value& root,
        const char* configKey,
        VehiclePropertyType propertyKey,
        std::unordered_map<VehiclePropertyType, std::string, EnumHash>& propertyMap);

    /**
     * Gets the attribute described by the specified @c VehiclePropertyType as a string. The value returned corresponds
     * to the name of the attribute as required for metrics.
     */
    std::string getPropertyAttributeForMetric(VehiclePropertyType property);
    std::shared_ptr<aace::engine::metrics::MetricEvent> generateVehiclePropertiesMetric(bool empty);

private:
    bool registerProperties();

private:
    std::unordered_map<VehiclePropertyType, std::string, EnumHash> m_vehiclePropertyMap;
    std::string m_operatingCountry;
    /// Record empty metric flag
    bool m_recordEmpty;
    /// Record full metric flag
    bool m_recordFull;
    /// Whether "aace.vehicle.info" is configured
    bool m_vehicleInfoConfigured;
};

}  // namespace vehicle
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_VEHICLE_VEHICLE_ENGINE_SERVICE_H
