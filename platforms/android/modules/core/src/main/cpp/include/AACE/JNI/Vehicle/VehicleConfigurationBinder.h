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

#ifndef AACE_JNI_VEHICLE_VEHICLE_CONFIGURATION_BINDER_H
#define AACE_JNI_VEHICLE_VEHICLE_CONFIGURATION_BINDER_H

#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/Vehicle/VehicleConfiguration.h>

namespace aace {
namespace jni {
namespace vehicle {

//
// JVehiclePropertyType
//

class JVehiclePropertyTypeConfig
        : public EnumConfiguration<aace::vehicle::config::VehicleConfiguration::VehiclePropertyType> {
public:
    using T = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;

    const char* getClassName() override {
        return "com/amazon/aace/vehicle/config/VehicleConfiguration$VehiclePropertyType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::MAKE, "MAKE"},
                {T::MODEL, "MODEL"},
                {T::YEAR, "YEAR"},
                {T::TRIM, "TRIM"},
                {T::GEOGRAPHY, "GEOGRAPHY"},
                {T::VERSION, "VERSION"},
                {T::OPERATING_SYSTEM, "OPERATING_SYSTEM"},
                {T::HARDWARE_ARCH, "HARDWARE_ARCH"},
                {T::LANGUAGE, "LANGUAGE"},
                {T::MICROPHONE, "MICROPHONE"},
                {T::VEHICLE_IDENTIFIER, "VEHICLE_IDENTIFIER"}};
    }
};

using JVehiclePropertyType =
    JEnum<aace::vehicle::config::VehicleConfiguration::VehiclePropertyType, JVehiclePropertyTypeConfig>;

//
// JVehicleProperty
//

class JVehicleProperty : public JObject {
public:
    JVehicleProperty(jobject obj) :
            JObject(obj, "com/amazon/aace/vehicle/config/VehicleConfiguration$VehicleProperty") {
    }

    aace::vehicle::config::VehicleConfiguration::VehicleProperty getVehicleProperty();

    static std::vector<aace::vehicle::config::VehicleConfiguration::VehicleProperty> convert(
        jobjectArray vehiclePropertyArrObj);
};
}  // namespace vehicle
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_VEHICLE_VEHICLE_CONFIGURATION_BINDER_H
