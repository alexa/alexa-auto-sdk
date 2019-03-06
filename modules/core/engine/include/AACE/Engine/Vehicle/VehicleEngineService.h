/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "AACE/Engine/Vehicle/VehiclePropertyInterface.h"
#include "AACE/Engine/Metrics/MetricEvent.h"

#include "AACE/Vehicle/VehicleConfiguration.h"

#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace vehicle {

struct EnumHash
{
    template <typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

class VehicleEngineService :
    public aace::engine::core::EngineService,
    public VehiclePropertyInterface,
    public std::enable_shared_from_this<VehicleEngineService> {
    
public:
    DESCRIBE("aace.vehicle",VERSION("1.0"))

public:
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;

private:
    VehicleEngineService( const aace::engine::core::ServiceDescription& description );

public:
    virtual ~VehicleEngineService() = default;
    
    // VehiclePropertyInterface
    std::string getVehicleProperty( VehiclePropertyType type ) override;
    
protected:
    bool initialize() override;
    bool setup() override;
    bool configure( const std::vector<std::shared_ptr<std::istream>>& configuration ) override;
    bool setProperty( const std::string& key, const std::string& value ) override;
    std::string getProperty( const std::string& key ) override;
private:
    bool configure( std::shared_ptr<std::istream> configuration );

    bool checkVehicleConfigProperty( rapidjson::Value& root, const char* key, bool warnIfMissing = true );
    std::string getVehicleConfigProperty( rapidjson::Value& root, const char* key, const char* defaultValue = "", bool warnIfMissing = true );
    std::string getVehiclePropertyAttribute( VehiclePropertyType property);
    std::shared_ptr<aace::engine::metrics::MetricEvent> generateVehiclePropertiesMetric();

private:
    std::unordered_map<VehiclePropertyType,std::string,EnumHash> m_vehiclePropertyMap;
    std::string m_operatingCountry;
    std::shared_ptr<aace::engine::metrics::MetricEvent> m_vehiclePropertiesMetric;
};

} // aace::engine::vehicle
} // aace::engine
} // aace

#endif // AACE_ENGINE_VEHICLE_VEHICLE_ENGINE_SERVICE_H
