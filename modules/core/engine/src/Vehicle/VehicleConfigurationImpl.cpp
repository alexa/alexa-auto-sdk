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

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Engine/Vehicle/VehicleConfigServiceInterface.h>
#include <AACE/Vehicle/VehicleConfiguration.h>

namespace aace {
namespace vehicle {
namespace config {

using namespace aace::engine::vehicle;
namespace json = aace::engine::utils::json;
using VehicleInfoPropertyType = VehicleConfiguration::VehicleInfoPropertyType;
using DeviceInfoPropertyType = VehicleConfiguration::DeviceInfoPropertyType;
using AppInfoPropertyType = VehicleConfiguration::AppInfoPropertyType;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.vehicle.config.VehicleConfigurationImpl");

std::string getVehicleInfoKey(VehicleInfoPropertyType property) {
    switch (property) {
        case VehicleInfoPropertyType::MAKE:
            return KEY_VEHICLE_INFO_MAKE;
        case VehicleInfoPropertyType::MODEL:
            return KEY_VEHICLE_INFO_MODEL;
        case VehicleInfoPropertyType::YEAR:
            return KEY_VEHICLE_INFO_YEAR;
        case VehicleInfoPropertyType::TRIM:
            return KEY_VEHICLE_INFO_TRIM;
        case VehicleInfoPropertyType::OPERATING_COUNTRY:
            return KEY_VEHICLE_INFO_OPERATING_COUNTRY;
        case VehicleInfoPropertyType::MICROPHONE_TYPE:
            return KEY_VEHICLE_INFO_MIC_TYPE;
        case VehicleInfoPropertyType::VEHICLE_IDENTIFIER:
            return KEY_VEHICLE_INFO_VEHICLE_ID;
        case VehicleInfoPropertyType::ENGINE_TYPE:
            return KEY_VEHICLE_INFO_ENGINE_TYPE;
        case VehicleInfoPropertyType::RSE_EMBEDDED_FIRE_TVS:
            return KEY_VEHICLE_INFO_RSE_FIRE_TVS;
    }
    return "";
}

std::string getDeviceInfoKey(DeviceInfoPropertyType property) {
    switch (property) {
        case DeviceInfoPropertyType::MANUFACTURER:
            return KEY_DEVICE_INFO_MANUFACTURER;
        case DeviceInfoPropertyType::MODEL:
            return KEY_DEVICE_INFO_MODEL;
        case DeviceInfoPropertyType::SERIAL_NUMBER:
            return KEY_DEVICE_INFO_SERIAL_NUMBER;
        case DeviceInfoPropertyType::PLATFORM:
            return KEY_DEVICE_INFO_PLATFORM;
        case DeviceInfoPropertyType::OS_VERSION:
            return KEY_DEVICE_INFO_OS_VERSION;
        case DeviceInfoPropertyType::HARDWARE_ARCH:
            return KEY_DEVICE_INFO_HARDWARE_ARCH;
    }
    return "";
}

std::string getAppInfoKey(AppInfoPropertyType property) {
    switch (property) {
        case AppInfoPropertyType::SOFTWARE_VERSION:
            return KEY_APP_INFO_SOFTWARE_VERSION;
    }
    return "";
}

std::shared_ptr<aace::core::config::EngineConfiguration> VehicleConfiguration::createVehicleInfoConfig(
    const std::vector<VehicleConfiguration::VehicleInfoProperty>& propertyList) {
    try {
        json::Value info = {};

        for (auto next : propertyList) {
            std::string name = getVehicleInfoKey(next.first);
            json::Value value = next.second;
            json::set(info, name, value);
        }

        // clang-format off
        json::Value config = {
            {"aace.vehicle",{
                {"vehicleInfo",info}
            }}
        };
        // clang-format on

        return aace::core::config::StreamConfiguration::create(json::toStream(config));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createVehicleInfoConfig failed").d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<aace::core::config::EngineConfiguration> VehicleConfiguration::createDeviceInfoConfig(
    const std::vector<VehicleConfiguration::DeviceInfoProperty>& propertyList) {
    try {
        json::Value info = {};

        for (auto next : propertyList) {
            std::string name = getDeviceInfoKey(next.first);
            json::Value value = next.second;
            json::set(info, name, value);
        }

        // clang-format off
        json::Value config = {
            {"aace.vehicle",{
                {"deviceInfo",info}
            }}
        };
        // clang-format on

        return aace::core::config::StreamConfiguration::create(json::toStream(config));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createDeviceInfoConfig failed").d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<aace::core::config::EngineConfiguration> VehicleConfiguration::createAppInfoConfig(
    const std::vector<VehicleConfiguration::AppInfoProperty>& propertyList) {
    try {
        json::Value info = {};

        for (auto next : propertyList) {
            std::string name = getAppInfoKey(next.first);
            json::Value value = next.second;
            json::set(info, name, value);
        }

        // clang-format off
        json::Value config = {
            {"aace.vehicle",{
                {"appInfo",info}
            }}
        };
        // clang-format on

        return aace::core::config::StreamConfiguration::create(json::toStream(config));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createAppInfoConfig failed").d("reason", ex.what()));
        return nullptr;
    }
}

}  // namespace config
}  // namespace vehicle
}  // namespace aace
