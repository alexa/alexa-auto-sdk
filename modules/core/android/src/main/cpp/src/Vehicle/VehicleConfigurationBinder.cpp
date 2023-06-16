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

#include <AACE/JNI/Core/EngineConfigurationBinder.h>
#include <AACE/JNI/Vehicle/VehicleConfigurationBinder.h>

// String to identify log entries orignating from this file
static const char TAG[] = "aace.jni.vehicle.config.VehicleConfigurationBinder";

// type aliases
using VehicleInfoProperty = aace::vehicle::config::VehicleConfiguration::VehicleInfoProperty;
using VehicleInfoPropertyType = aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType;
using DeviceInfoProperty = aace::vehicle::config::VehicleConfiguration::DeviceInfoProperty;
using DeviceInfoPropertyType = aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType;
using AppInfoProperty = aace::vehicle::config::VehicleConfiguration::AppInfoProperty;
using AppInfoPropertyType = aace::vehicle::config::VehicleConfiguration::AppInfoPropertyType;

namespace aace {
namespace jni {
namespace vehicle {

//
// JVehicleInfoProperty
//

VehicleInfoProperty JVehicleInfoProperty::getVehicleInfoProperty() {
    try_with_context {
        jobject propertyTypeObj;
        ThrowIfNot(
            invoke(
                "getType",
                "()Lcom/amazon/aace/vehicle/config/VehicleConfiguration$VehicleInfoPropertyType;",
                &propertyTypeObj),
            "invokeMethodFailed");

        VehicleInfoPropertyType checkedPropertyTypeObj;
        ThrowIfNot(
            JVehicleInfoPropertyType::checkType(propertyTypeObj, &checkedPropertyTypeObj), "invalidPropertyType");

        jstring propertyValueObj;
        ThrowIfNot(invoke("getValue", "()Ljava/lang/String;", &propertyValueObj), "invokeMethodFailed");

        return {checkedPropertyTypeObj, JString(propertyValueObj).toStdStr()};
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getVehicleInfoProperty", ex.what());
        return {};
    }
}

std::vector<VehicleInfoProperty> JVehicleInfoProperty::convert(jobjectArray vehiclePropertyArrObj) {
    try_with_context {
        // convert java vehicle property
        std::vector<VehicleInfoProperty> vehicleProperties;
        JObjectArray arr(vehiclePropertyArrObj);
        jobject next;

        for (int j = 0; j < arr.size(); j++) {
            ThrowIfNot(arr.getAt(j, &next), "getArrayValueFailed");
            vehicleProperties.push_back(aace::jni::vehicle::JVehicleInfoProperty(next).getVehicleInfoProperty());
        }

        return vehicleProperties;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "convert", ex.what());
        return {};
    }
}

//
// JDeviceInfoProperty
//

DeviceInfoProperty JDeviceInfoProperty::getDeviceInfoProperty() {
    try_with_context {
        jobject propertyTypeObj;
        ThrowIfNot(
            invoke(
                "getType",
                "()Lcom/amazon/aace/vehicle/config/VehicleConfiguration$DeviceInfoPropertyType;",
                &propertyTypeObj),
            "invokeMethodFailed");

        DeviceInfoPropertyType checkedPropertyTypeObj;
        ThrowIfNot(JDeviceInfoPropertyType::checkType(propertyTypeObj, &checkedPropertyTypeObj), "invalidPropertyType");

        jstring propertyValueObj;
        ThrowIfNot(invoke("getValue", "()Ljava/lang/String;", &propertyValueObj), "invokeMethodFailed");

        return {checkedPropertyTypeObj, JString(propertyValueObj).toStdStr()};
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getDeviceInfoProperty", ex.what());
        return {};
    }
}

std::vector<DeviceInfoProperty> JDeviceInfoProperty::convert(jobjectArray vehiclePropertyArrObj) {
    try_with_context {
        // convert java vehicle property
        std::vector<DeviceInfoProperty> vehicleProperties;
        JObjectArray arr(vehiclePropertyArrObj);
        jobject next;

        for (int j = 0; j < arr.size(); j++) {
            ThrowIfNot(arr.getAt(j, &next), "getArrayValueFailed");
            vehicleProperties.push_back(aace::jni::vehicle::JDeviceInfoProperty(next).getDeviceInfoProperty());
        }

        return vehicleProperties;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "convert", ex.what());
        return {};
    }
}

//
// JAppInfoProperty
//

AppInfoProperty JAppInfoProperty::getAppInfoProperty() {
    try_with_context {
        jobject propertyTypeObj;
        ThrowIfNot(
            invoke(
                "getType",
                "()Lcom/amazon/aace/vehicle/config/VehicleConfiguration$AppInfoPropertyType;",
                &propertyTypeObj),
            "invokeMethodFailed");

        AppInfoPropertyType checkedPropertyTypeObj;
        ThrowIfNot(JAppInfoPropertyType::checkType(propertyTypeObj, &checkedPropertyTypeObj), "invalidPropertyType");

        jstring propertyValueObj;
        ThrowIfNot(invoke("getValue", "()Ljava/lang/String;", &propertyValueObj), "invokeMethodFailed");

        return {checkedPropertyTypeObj, JString(propertyValueObj).toStdStr()};
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getAppInfoProperty", ex.what());
        return {};
    }
}

std::vector<AppInfoProperty> JAppInfoProperty::convert(jobjectArray vehiclePropertyArrObj) {
    try_with_context {
        // convert java vehicle property
        std::vector<AppInfoProperty> vehicleProperties;
        JObjectArray arr(vehiclePropertyArrObj);
        jobject next;

        for (int j = 0; j < arr.size(); j++) {
            ThrowIfNot(arr.getAt(j, &next), "getArrayValueFailed");
            vehicleProperties.push_back(aace::jni::vehicle::JAppInfoProperty(next).getAppInfoProperty());
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
            aace::jni::vehicle::JVehicleInfoProperty::convert(arr));
        ThrowIfNull(config, "createVehicleInfoConfigBinderFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_vehicle_config_VehicleConfiguration_createVehicleInfoConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_vehicle_config_VehicleConfiguration_createDeviceInfoConfigBinder(
    JNIEnv* env,
    jobject obj,
    jobjectArray arr) {
    try {
        auto config = aace::vehicle::config::VehicleConfiguration::createDeviceInfoConfig(
            aace::jni::vehicle::JDeviceInfoProperty::convert(arr));
        ThrowIfNull(config, "createDeviceInfoConfigBinderFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_vehicle_config_VehicleConfiguration_createDeviceInfoConfigBinder", ex.what());
        return 0;
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_vehicle_config_VehicleConfiguration_createAppInfoConfigBinder(
    JNIEnv* env,
    jobject obj,
    jobjectArray arr) {
    try {
        auto config = aace::vehicle::config::VehicleConfiguration::createAppInfoConfig(
            aace::jni::vehicle::JAppInfoProperty::convert(arr));
        ThrowIfNull(config, "createAppInfoConfigBinderFailed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_vehicle_config_VehicleConfiguration_createAppInfoConfigBinder", ex.what());
        return 0;
    }
}
}