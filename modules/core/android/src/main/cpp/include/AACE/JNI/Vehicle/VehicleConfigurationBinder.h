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

#ifndef AACE_JNI_VEHICLE_VEHICLE_CONFIGURATION_BINDER_H
#define AACE_JNI_VEHICLE_VEHICLE_CONFIGURATION_BINDER_H

#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/Vehicle/VehicleConfiguration.h>

namespace aace {
namespace jni {
namespace vehicle {

//
// JVehicleInfoPropertyType
//

class JVehicleInfoPropertyTypeConfig
        : public EnumConfiguration<aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType> {
public:
    using T = aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType;

    const char* getClassName() override {
        return "com/amazon/aace/vehicle/config/VehicleConfiguration$VehicleInfoPropertyType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::MAKE, "MAKE"},
                {T::MODEL, "MODEL"},
                {T::YEAR, "YEAR"},
                {T::TRIM, "TRIM"},
                {T::OPERATING_COUNTRY, "OPERATING_COUNTRY"},
                {T::MICROPHONE_TYPE, "MICROPHONE_TYPE"},
                {T::VEHICLE_IDENTIFIER, "VEHICLE_IDENTIFIER"},
                {T::ENGINE_TYPE, "ENGINE_TYPE"},
                {T::RSE_EMBEDDED_FIRE_TVS, "RSE_EMBEDDED_FIRE_TVS"}};
    }
};

using JVehicleInfoPropertyType =
    JEnum<aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType, JVehicleInfoPropertyTypeConfig>;

//
// JVehicleInfoProperty
//

class JVehicleInfoProperty : public JObject {
public:
    JVehicleInfoProperty(jobject obj) :
            JObject(obj, "com/amazon/aace/vehicle/config/VehicleConfiguration$VehicleInfoProperty") {
    }

    aace::vehicle::config::VehicleConfiguration::VehicleInfoProperty getVehicleInfoProperty();

    static std::vector<aace::vehicle::config::VehicleConfiguration::VehicleInfoProperty> convert(
        jobjectArray vehiclePropertyArrObj);
};

//
// JDeviceInfoPropertyType
//

class JDeviceInfoPropertyTypeConfig
        : public EnumConfiguration<aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType> {
public:
    using T = aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType;

    const char* getClassName() override {
        return "com/amazon/aace/vehicle/config/VehicleConfiguration$DeviceInfoPropertyType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::MANUFACTURER, "MANUFACTURER"},
                {T::MODEL, "MODEL"},
                {T::SERIAL_NUMBER, "SERIAL_NUMBER"},
                {T::PLATFORM, "PLATFORM"},
                {T::OS_VERSION, "OS_VERSION"},
                {T::HARDWARE_ARCH, "HARDWARE_ARCH"}};
    }
};

using JDeviceInfoPropertyType =
    JEnum<aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType, JDeviceInfoPropertyTypeConfig>;

//
// JDeviceInfoProperty
//

class JDeviceInfoProperty : public JObject {
public:
    JDeviceInfoProperty(jobject obj) :
            JObject(obj, "com/amazon/aace/vehicle/config/VehicleConfiguration$DeviceInfoProperty") {
    }

    aace::vehicle::config::VehicleConfiguration::DeviceInfoProperty getDeviceInfoProperty();

    static std::vector<aace::vehicle::config::VehicleConfiguration::DeviceInfoProperty> convert(
        jobjectArray vehiclePropertyArrObj);
};

//
// JAppInfoPropertyType
//

class JAppInfoPropertyTypeConfig
        : public EnumConfiguration<aace::vehicle::config::VehicleConfiguration::AppInfoPropertyType> {
public:
    using T = aace::vehicle::config::VehicleConfiguration::AppInfoPropertyType;

    const char* getClassName() override {
        return "com/amazon/aace/vehicle/config/VehicleConfiguration$AppInfoPropertyType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::SOFTWARE_VERSION, "SOFTWARE_VERSION"}};
    }
};

using JAppInfoPropertyType =
    JEnum<aace::vehicle::config::VehicleConfiguration::AppInfoPropertyType, JAppInfoPropertyTypeConfig>;

//
// JAppInfoProperty
//

class JAppInfoProperty : public JObject {
public:
    JAppInfoProperty(jobject obj) :
            JObject(obj, "com/amazon/aace/vehicle/config/VehicleConfiguration$AppInfoProperty") {
    }

    aace::vehicle::config::VehicleConfiguration::AppInfoProperty getAppInfoProperty();

    static std::vector<aace::vehicle::config::VehicleConfiguration::AppInfoProperty> convert(
        jobjectArray vehiclePropertyArrObj);
};

}  // namespace vehicle
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_VEHICLE_VEHICLE_CONFIGURATION_BINDER_H
