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
#include <stdexcept>

#include <nlohmann/json.hpp>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h>
#include <AACE/Engine/Storage/LocalStorageInterface.h>
#include <AACE/Engine/Vehicle/VehicleEngineService.h>
#include <AACE/Vehicle/VehicleProperties.h>

namespace aace {
namespace engine {
namespace vehicle {

using json = nlohmann::json;

/// String to identify log entries originating from this file
static const std::string TAG("aace.engine.vehicle.VehicleEngineService");

// register the service
REGISTER_SERVICE(VehicleEngineService)

VehicleEngineService::VehicleEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool VehicleEngineService::initialize() {
    try {
        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "PropertyManagerServiceInterface is null");

        propertyManager->registerProperty(aace::engine::propertyManager::PropertyDescription(
            aace::vehicle::property::OPERATING_COUNTRY,
            std::bind(
                &VehicleEngineService::setProperty_operatingCountry,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4),
            std::bind(&VehicleEngineService::getProperty_operatingCountry, this)));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool VehicleEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        json config = json::parse(*configuration);
        AACE_DEBUG(LX(TAG).sensitive("config", config.dump()));
        ThrowIfNot(config.contains("vehicleInfo"), "Missing 'vehicleInfo'");
        json vehicleInfo = config.at("vehicleInfo");
        ThrowIfNot(vehicleInfo.is_object(), "vehicleInfo must be an object");
        ThrowIfNot(vehicleInfo.contains(KEY_VEHICLE_INFO_MAKE), "Missing key " + KEY_VEHICLE_INFO_MAKE);
        ThrowIfNot(vehicleInfo.contains(KEY_VEHICLE_INFO_MODEL), "Missing key " + KEY_VEHICLE_INFO_MODEL);
        ThrowIfNot(vehicleInfo.contains(KEY_VEHICLE_INFO_YEAR), "Missing key " + KEY_VEHICLE_INFO_YEAR);
        std::string make = vehicleInfo.at(KEY_VEHICLE_INFO_MAKE);
        std::string model = vehicleInfo.at(KEY_VEHICLE_INFO_MODEL);
        std::string year = vehicleInfo.at(KEY_VEHICLE_INFO_YEAR);

        ThrowIfNot(config.contains("deviceInfo"), "Missing 'deviceInfo'");
        json deviceInfo = config.at("deviceInfo");
        ThrowIfNot(deviceInfo.is_object(), "deviceInfo must be an object");
        ThrowIfNot(deviceInfo.contains(KEY_DEVICE_INFO_SERIAL_NUMBER), "Missing key " + KEY_DEVICE_INFO_SERIAL_NUMBER);
        std::string serialNumber = deviceInfo.at(KEY_DEVICE_INFO_SERIAL_NUMBER);

        std::string vehicleId;
        std::string trim;
        std::string mic;
        std::string country;
        std::string engineType;
        std::string rseTvs;
        if (vehicleInfo.contains(KEY_VEHICLE_INFO_VEHICLE_ID)) {
            vehicleId = vehicleInfo.at(KEY_VEHICLE_INFO_VEHICLE_ID);
        }
        if (vehicleInfo.contains(KEY_VEHICLE_INFO_TRIM)) {
            trim = vehicleInfo.at(KEY_VEHICLE_INFO_TRIM);
        }
        if (vehicleInfo.contains(KEY_VEHICLE_INFO_MIC_TYPE)) {
            mic = vehicleInfo.at(KEY_VEHICLE_INFO_MIC_TYPE);
        }
        if (vehicleInfo.contains(KEY_VEHICLE_INFO_OPERATING_COUNTRY)) {
            country = vehicleInfo.at(KEY_VEHICLE_INFO_OPERATING_COUNTRY);
        }
        if (vehicleInfo.contains(KEY_VEHICLE_INFO_ENGINE_TYPE)) {
            engineType = vehicleInfo.at(KEY_VEHICLE_INFO_ENGINE_TYPE);
        }
        if (vehicleInfo.contains(KEY_VEHICLE_INFO_RSE_FIRE_TVS)) {
            rseTvs = vehicleInfo.at(KEY_VEHICLE_INFO_RSE_FIRE_TVS);
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        m_vehicleInfoMap.insert({KEY_VEHICLE_INFO_MAKE, make});
        m_vehicleInfoMap.insert({KEY_VEHICLE_INFO_MODEL, model});
        m_vehicleInfoMap.insert({KEY_VEHICLE_INFO_YEAR, year});
        m_vehicleInfoMap.insert({KEY_VEHICLE_INFO_VEHICLE_ID, vehicleId});
        m_vehicleInfoMap.insert({KEY_VEHICLE_INFO_TRIM, trim});
        m_vehicleInfoMap.insert({KEY_VEHICLE_INFO_MIC_TYPE, mic});
        m_vehicleInfoMap.insert({KEY_VEHICLE_INFO_OPERATING_COUNTRY, country});
        m_vehicleInfoMap.insert({KEY_VEHICLE_INFO_ENGINE_TYPE, engineType});
        m_vehicleInfoMap.insert({KEY_VEHICLE_INFO_RSE_FIRE_TVS, rseTvs});

        std::string appVersion;
        if (config.contains("appInfo")) {
            json appInfo = config.at("appInfo");
            ThrowIfNot(appInfo.is_object(), "appInfo must be an object");
            if (appInfo.contains(KEY_APP_INFO_SOFTWARE_VERSION)) {
                appVersion = appInfo.at(KEY_APP_INFO_SOFTWARE_VERSION);
            }
        }
        m_appInfoMap.insert({KEY_APP_INFO_SOFTWARE_VERSION, appVersion});

        std::string manufacturer;
        std::string deviceModel;
        std::string osVersion;
        std::string hardwareArch;
        std::string platform;

        if (deviceInfo.contains(KEY_DEVICE_INFO_MANUFACTURER)) {
            manufacturer = deviceInfo.at(KEY_DEVICE_INFO_MANUFACTURER);
        }
        if (deviceInfo.contains(KEY_DEVICE_INFO_MODEL)) {
            deviceModel = deviceInfo.at(KEY_DEVICE_INFO_MODEL);
        }
        if (deviceInfo.contains(KEY_DEVICE_INFO_OS_VERSION)) {
            osVersion = deviceInfo.at(KEY_DEVICE_INFO_OS_VERSION);
        }
        if (deviceInfo.contains(KEY_DEVICE_INFO_HARDWARE_ARCH)) {
            hardwareArch = deviceInfo.at(KEY_DEVICE_INFO_HARDWARE_ARCH);
        }
        if (deviceInfo.contains(KEY_DEVICE_INFO_PLATFORM)) {
            platform = deviceInfo.at(KEY_DEVICE_INFO_PLATFORM);
        }
        m_deviceInfoMap.insert({KEY_DEVICE_INFO_SERIAL_NUMBER, serialNumber});
        m_deviceInfoMap.insert({KEY_DEVICE_INFO_MANUFACTURER, manufacturer});
        m_deviceInfoMap.insert({KEY_DEVICE_INFO_MODEL, deviceModel});
        m_deviceInfoMap.insert({KEY_DEVICE_INFO_OS_VERSION, osVersion});
        m_deviceInfoMap.insert({KEY_DEVICE_INFO_HARDWARE_ARCH, hardwareArch});
        m_deviceInfoMap.insert({KEY_DEVICE_INFO_SERIAL_NUMBER, serialNumber});
        m_deviceInfoMap.insert({KEY_DEVICE_INFO_PLATFORM, platform});

        ThrowIfNot(
            registerServiceInterface<VehicleConfigServiceInterface>(shared_from_this()),
            "register VehicleConfigServiceInterface failed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "Configure failed").d("reason", ex.what()));
        return false;
    }
}

bool VehicleEngineService::configure() {
    AACE_ERROR(LX(TAG, "Configure failed").d("reason", "aace.vehicle config is required"));
    return false;
}

bool VehicleEngineService::setProperty_operatingCountry(
    const std::string& value,
    bool& changed,
    bool& async,
    const SetPropertyResultCallback& callbackFunction) {
    try {
        AACE_INFO(LX(TAG).sensitive("value", value));
        std::lock_guard<std::mutex> lock(m_mutex);
        async = false;
        auto currentCountryIt = m_vehicleInfoMap.find(KEY_VEHICLE_INFO_OPERATING_COUNTRY);
        ThrowIf(currentCountryIt == m_vehicleInfoMap.end(), "Could not retrieve current value");
        const std::string current = currentCountryIt->second;
        ReturnIf(value == current, true);
        currentCountryIt->second = value;
        changed = true;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "setProperty_operatingCountry failed").d("reason", ex.what()).sensitive("value", value));
        return false;
    }
}

std::string VehicleEngineService::getProperty_operatingCountry() {
    AACE_INFO(LX(TAG));
    std::lock_guard<std::mutex> lock(m_mutex);
    auto currentCountryIt = m_vehicleInfoMap.find(KEY_VEHICLE_INFO_OPERATING_COUNTRY);
    if (currentCountryIt == m_vehicleInfoMap.end()) {
        AACE_WARN(LX(TAG).m("Failed to retrieve current operating country"));
        return "";
    }
    return currentCountryIt->second;
}

std::string VehicleEngineService::getVehicleInfoValue(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_vehicleInfoMap.find(key);
    if (it == m_vehicleInfoMap.end()) {
        AACE_WARN(LX(TAG).d("Failed to retrieve value for key", key));
        throw std::invalid_argument("Invalid key name");
    }
    return it->second;
}
std::string VehicleEngineService::getAppInfoValue(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_appInfoMap.find(key);
    if (it == m_appInfoMap.end()) {
        AACE_WARN(LX(TAG).d("Failed to retrieve value for key", key));
        throw std::invalid_argument("Invalid key name");
    }
    return it->second;
}
std::string VehicleEngineService::getDeviceInfoValue(const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_deviceInfoMap.find(key);
    if (it == m_deviceInfoMap.end()) {
        AACE_WARN(LX(TAG).d("Failed to retrieve value for key", key));
        throw std::invalid_argument("Invalid key name: " + key);
    }
    return it->second;
}

}  // namespace vehicle
}  // namespace engine
}  // namespace aace
