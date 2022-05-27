/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Storage/LocalStorageInterface.h"
#include "AACE/Engine/Utils/JSON/JSON.h"
#include "AACE/Engine/Vehicle/VehicleEngineService.h"
#include "AACE/Engine/Vehicle/VehiclePropertyInterface.h"
#include "AACE/Engine/Utils/String/StringUtils.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Vehicle/VehicleProperties.h"

namespace aace {
namespace engine {
namespace vehicle {

// json namespace alias
namespace json = aace::engine::utils::json;

/// String to identify log entries originating from this file
static const std::string TAG("aace.vehicle.VehicleEngineService");
/// Name of the table used for the local storage database
static const std::string VEHICLE_SERVICE_LOCAL_STORAGE_TABLE = "aace.vehicle";

// register the service
REGISTER_SERVICE(VehicleEngineService)

VehicleEngineService::VehicleEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description), m_recordFull(true), m_vehicleInfoConfigured(false) {
}

std::string VehicleEngineService::getVehicleProperty(VehiclePropertyType type) {
    auto it = m_vehiclePropertyMap.find(type);
    return it != m_vehiclePropertyMap.end() ? it->second : "";
}

VehiclePropertyMap VehicleEngineService::getVehicleProperties() {
    return m_vehiclePropertyMap;
}

std::string VehicleEngineService::getPropertyAttributeForMetric(VehiclePropertyType property) {
    switch (property) {
        case VehiclePropertyType::MAKE:
            return "Make";
        case VehiclePropertyType::MODEL:
            return "Model";
        case VehiclePropertyType::YEAR:
            return "Year";
        case VehiclePropertyType::TRIM:
            return "Trim";
        case VehiclePropertyType::GEOGRAPHY:
            return "Geography";
        case VehiclePropertyType::VERSION:
            return "SWVersion";
        case VehiclePropertyType::OPERATING_SYSTEM:
            return "OS";
        case VehiclePropertyType::HARDWARE_ARCH:
            return "HW";
        case VehiclePropertyType::LANGUAGE:
            return "Language";
        case VehiclePropertyType::MICROPHONE:
            return "Microphone";
        case VehiclePropertyType::VEHICLE_IDENTIFIER:
            return "VehicleIdentifier";
        case VehiclePropertyType::ENGINE_TYPE:
            return "engineType";
        case VehiclePropertyType::RSE_EMBEDDED_FIRETVS:
            return "rseEmbeddedFireTvs";
    }
    return "";
}

bool VehicleEngineService::initialize() {
    try {
        ThrowIfNot(
            registerServiceInterface<VehiclePropertyInterface>(shared_from_this()),
            "registerVehiclePropertyInterfaceFailed");
        ThrowIfNot(registerProperties(), "registerPropertiesFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool VehicleEngineService::registerProperties() {
    try {
        // get the property engine service interface from the property manager service
        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

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

bool VehicleEngineService::setup() {
    try {
        // get the operating country from the settings
        auto localStorage =
            getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>("aace.storage");

        if (localStorage != nullptr &&
            localStorage->containsKey(VEHICLE_SERVICE_LOCAL_STORAGE_TABLE, "operatingCountry")) {
            m_operatingCountry =
                localStorage->get(VEHICLE_SERVICE_LOCAL_STORAGE_TABLE, "operatingCountry", m_operatingCountry);
        }

        // warn if the vehicle property map has not been configured
        if (m_vehiclePropertyMap.empty()) {
            AACE_WARN(LX(TAG, "setup").m("vehicleInfoNotConfigured"));
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "setup").d("reason", ex.what()));
        return false;
    }

    return true;
}

void VehicleEngineService::getVehicleConfigProperty(
    json::Value& root,
    const char* configKey,
    VehiclePropertyType propertyKey,
    std::unordered_map<VehiclePropertyType, std::string, EnumHash>& propertyMap) {
    try {
        ReturnIf(json::get(root, configKey, json::Type::string) == nullptr);
        auto value = json::get(root, configKey);
        AACE_DEBUG(LX(TAG, "VehicleProperty").d(configKey, value));
        propertyMap[propertyKey] = value;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool VehicleEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        auto root = aace::engine::utils::json::toJson(configuration);
        ThrowIfNull(root, "parseConfigurationFailed");

        auto info = json::get(root, "/info", json::Type::object);
        if (info != nullptr) {
            getVehicleConfigProperty(info, "make", VehiclePropertyType::MAKE, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "model", VehiclePropertyType::MODEL, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "year", VehiclePropertyType::YEAR, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "trim", VehiclePropertyType::TRIM, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "geography", VehiclePropertyType::GEOGRAPHY, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "version", VehiclePropertyType::VERSION, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "os", VehiclePropertyType::OPERATING_SYSTEM, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "arch", VehiclePropertyType::HARDWARE_ARCH, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "language", VehiclePropertyType::LANGUAGE, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "microphone", VehiclePropertyType::MICROPHONE, m_vehiclePropertyMap);
            getVehicleConfigProperty(
                info, "vehicleIdentifier", VehiclePropertyType::VEHICLE_IDENTIFIER, m_vehiclePropertyMap);
            getVehicleConfigProperty(info, "engineType", VehiclePropertyType::ENGINE_TYPE, m_vehiclePropertyMap);
            getVehicleConfigProperty(
                info, "rseEmbeddedFireTvs", VehiclePropertyType::RSE_EMBEDDED_FIRETVS, m_vehiclePropertyMap);
            m_vehicleInfoConfigured = true;
        }

        auto operatingCountry = json::get(root, "/operatingCountry", json::Type::string);
        if (operatingCountry != nullptr) {
            m_operatingCountry = operatingCountry;
        }

        // Register the vehicle engine service
        ThrowIfNot(
            registerServiceInterface<VehicleEngineService>(shared_from_this()), "registerVehicleEngineServiceFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}

bool VehicleEngineService::isVehicleInfoConfigured() {
    return m_vehicleInfoConfigured;
}

bool VehicleEngineService::setProperty_operatingCountry(
    const std::string& value,
    bool& changed,
    bool& async,
    const SetPropertyResultCallback& callbackFunction) {
    try {
        AACE_INFO(LX(TAG).sensitive("value", value));

        ReturnIf(aace::engine::utils::string::equal(value, m_operatingCountry), true);

        auto localStorage =
            getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>("aace.storage");

        ThrowIfNull(localStorage, "localStorageInterfaceInvalid");
        ThrowIfNot(
            localStorage->put(VEHICLE_SERVICE_LOCAL_STORAGE_TABLE, "operatingCountry", value), "setLocalStorageFailed");

        m_operatingCountry = value;
        changed = true;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).sensitive("value", value));
        return false;
    }
}

std::string VehicleEngineService::getProperty_operatingCountry() {
    AACE_INFO(LX(TAG));
    return m_operatingCountry;
}

std::shared_ptr<aace::engine::metrics::MetricEvent> VehicleEngineService::generateVehiclePropertiesMetric() {
    std::string program = "AlexaAuto_Vehicle";
    std::string source = "VehicleConfiguration";
    std::shared_ptr<aace::engine::metrics::MetricEvent> currentMetric =
        std::shared_ptr<aace::engine::metrics::MetricEvent>(new aace::engine::metrics::MetricEvent(program, source));
    AACE_INFO(LX(TAG, "generateMetric").m("Added vehicle properties"));
    for (auto itr : m_vehiclePropertyMap) {
        VehiclePropertyType property = itr.first;
        std::string dataPointName = getPropertyAttributeForMetric(property);
        std::string dataPointValue = itr.second;

        // sanitize any delimiter characters from dataPointValue to maintain metric formatting
        char delimiters[] = ";=,:";
        for (char delimiter : delimiters) {
            std::replace(dataPointValue.begin(), dataPointValue.end(), delimiter, '-');
        }

        currentMetric->addString(dataPointName, dataPointValue);
    }

    return currentMetric;
}

/**
 * Record vehicle metric with no data points (empty) or with all data points (full).
 * Only necessary to emit the metric once for empty or full.
 */
void VehicleEngineService::record() {
    if (m_recordFull) {
        auto metric = generateVehiclePropertiesMetric();
        AACE_INFO(LX(TAG, "record").m("Recording metric with vehicle information"));
        metric->record();
        m_recordFull = false;
    }
}

}  // namespace vehicle
}  // namespace engine
}  // namespace aace
