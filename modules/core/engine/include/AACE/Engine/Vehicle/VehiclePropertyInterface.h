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

#ifndef AACE_ENGINE_VEHICLE_INTERFACE_VEHICLE_PROPERTY_INTERFACE_H
#define AACE_ENGINE_VEHICLE_INTERFACE_VEHICLE_PROPERTY_INTERFACE_H

#include "AACE/Vehicle/VehicleConfiguration.h"

namespace aace {
namespace engine {
namespace vehicle {

class VehiclePropertyInterface {
public:
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;

    virtual ~VehiclePropertyInterface();
    
    virtual std::string getVehicleProperty( VehiclePropertyType type ) = 0;
};

} // aace::engine::vehicle
} // aace::engine
} // aace

#endif // AACE_ENGINE_VEHICLE_INTERFACE_VEHICLE_PROPERTY_INTERFACE_H
