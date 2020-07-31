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

#include <AACE/JNI/Core/EngineConfigurationBinder.h>
#include <AACE/JNI/Vehicle/VehicleConfigurationBinder.h>

// String to identify log entries orignating from this file
static const char TAG[] = "aace.jni.vehicle.config.VehicleConfigurationBinder";

// type aliases
using VehicleProperty = aace::vehicle::config::VehicleConfiguration::VehicleProperty;
using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;

namespace aace {
namespace jni {
namespace vehicle {

//
// JVehicleProperty
//

VehicleProperty JVehicleProperty::getVehicleProperty() {
    try_with_context {
        jobject propertyTypeObj;
        ThrowIfNot(
            invoke(
                "getType",
                "()Lcom/amazon/aace/vehicle/config/VehicleConfiguration$VehiclePropertyType;",
                &propertyTypeObj),
            "invokeMethodFailed");

        VehiclePropertyType checkedPropertyTypeObj;
        ThrowIfNot(JVehiclePropertyType::checkType(propertyTypeObj, &checkedPropertyTypeObj), "invalidPropertyType");

        jstring propertyValueObj;
        ThrowIfNot(invoke("getValue", "()Ljava/lang/String;", &propertyValueObj), "invokeMethodFailed");

        return {checkedPropertyTypeObj, JString(propertyValueObj).toStdStr()};
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getVehicleProperty", ex.what());
        return {};
    }
}

std::vector<VehicleProperty> JVehicleProperty::convert(jobjectArray vehiclePropertyArrObj) {
    try_with_context {
        // convert java vehicle property
        std::vector<VehicleProperty> vehicleProperties;
        JObjectArray arr(vehiclePropertyArrObj);
        jobject next;

        for (int j = 0; j < arr.size(); j++) {
            ThrowIfNot(arr.getAt(j, &next), "getArrayValueFailed");
            vehicleProperties.push_back(aace::jni::vehicle::JVehicleProperty(next).getVehicleProperty());
        }

        return vehicleProperties;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "convert", ex.what());
        return {};
    }
}

}  // namespace vehicle
}  // namespace jni
}  // namespace aace

// JNI

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_vehicle_config_VehicleConfiguration_createVehicleInfoConfigBinder(
    JNIEnv* env,
    jobject obj,
    jobjectArray arr) {
    try {
        auto config = aace::vehicle::config::VehicleConfiguration::createVehicleInfoConfig(
            aace::jni::vehicle::JVehicleProperty::convert(arr));
        ThrowIfNull(config, "createVehicleInfoConfigBinderFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_vehicle_config_VehicleConfiguration_createVehicleInfoConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_vehicle_config_VehicleConfiguration_createOperatingCountryConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring operatingCountry) {
    try {
        auto config = aace::vehicle::config::VehicleConfiguration::createOperatingCountryConfig(
            JString(operatingCountry).toStdStr());
        ThrowIfNull(config, "createOperatingCountryConfigFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG,
            "Java_com_amazon_aace_vehicle_config_VehicleConfiguration_createOperatingCountryConfigBinder",
            ex.what());
        return 0;
    }
}
}