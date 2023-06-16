/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <mutex>
#include <unordered_map>

#include <AACE/Engine/Core/EngineService.h>
#include <AACE/Engine/PropertyManager/PropertyManagerEngineService.h>
#include <AACE/Engine/Vehicle/VehicleConfigServiceInterface.h>

namespace aace {
namespace engine {
namespace vehicle {

class VehicleEngineService
        : public aace::engine::core::EngineService
        , public VehicleConfigServiceInterface
        , public std::enable_shared_from_this<VehicleEngineService> {
public:
    DESCRIBE("aace.vehicle", VERSION("1.0"), DEPENDS(aace::engine::propertyManager::PropertyManagerEngineService))

private:
    VehicleEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~VehicleEngineService() = default;

    using SetPropertyResultCallback = std::function<void(const std::string&, const std::string&, const std::string&)>;

    /// @name VehicleConfigServiceInterface methods
    /// @{
    std::string getVehicleInfoValue(const std::string& key) override;
    std::string getAppInfoValue(const std::string& key) override;
    std::string getDeviceInfoValue(const std::string& key) override;
    /// @}

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
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool configure() override;
    /// @}

private:
    /// Serializes access to members
    std::mutex m_mutex;
    /// Holds the values from aace.vehicle.vehicleInfo
    std::unordered_map<std::string, std::string> m_vehicleInfoMap;
    /// Holds the values from aace.vehicle.appInfo
    std::unordered_map<std::string, std::string> m_appInfoMap;
    /// Holds the values from aace.vehicle.deviceInfo
    std::unordered_map<std::string, std::string> m_deviceInfoMap;
};

}  // namespace vehicle
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_VEHICLE_VEHICLE_ENGINE_SERVICE_H
