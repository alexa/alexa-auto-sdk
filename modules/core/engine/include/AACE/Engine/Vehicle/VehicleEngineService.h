/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "AACE/Vehicle/VehicleConfiguration.h"

#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace vehicle {

class VehicleEngineService : public aace::engine::core::EngineService {
public:
    DESCRIBE("aace.vehicle",VERSION("1.0"))

private:
    VehicleEngineService( const aace::engine::core::ServiceDescription& description );

public:
    virtual ~VehicleEngineService() = default;
    
protected:
    bool configure( const std::vector<std::shared_ptr<std::istream>>& configuration ) override;
    
private:
    bool configure( std::shared_ptr<std::istream> configuration );
    
    bool checkVehicleConfigProperty( rapidjson::Value& root, const char* key, bool warnIfMissing = true );
    std::string getVehicleConfigProperty( rapidjson::Value& root, const char* key, const char* defaultValue = "", bool warnIfMissing = true );
    
private:
    std::unordered_map<std::string,std::string> m_vehiclePropertyMap;
};

} // aace::engine::vehicle
} // aace::engine
} // aace

#endif // AACE_ENGINE_VEHICLE_VEHICLE_ENGINE_SERVICE_H
