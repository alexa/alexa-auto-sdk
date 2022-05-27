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
#include "AACE/Vehicle/VehicleConfiguration.h"
#include "AACE/Engine/Utils/JSON/JSON.h"

namespace aace {
namespace vehicle {
namespace config {

// json namespace alias
namespace json = aace::engine::utils::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.vehicle.config.VehicleConfiguationImpl");

// VehicleProperty alias
using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;

std::string getVehiclePropertyAttribute(VehiclePropertyType property) {
    switch (property) {
        case VehiclePropertyType::MAKE:
            return "make";
        case VehiclePropertyType::MODEL:
            return "model";
        case VehiclePropertyType::YEAR:
            return "year";
        case VehiclePropertyType::TRIM:
            return "trim";
        case VehiclePropertyType::GEOGRAPHY:
            return "geography";
        case VehiclePropertyType::VERSION:
            return "version";
        case VehiclePropertyType::OPERATING_SYSTEM:
            return "os";
        case VehiclePropertyType::HARDWARE_ARCH:
            return "arch";
        case VehiclePropertyType::LANGUAGE:
            return "language";
        case VehiclePropertyType::MICROPHONE:
            return "microphone";
        case VehiclePropertyType::VEHICLE_IDENTIFIER:
            return "vehicleIdentifier";
        case VehiclePropertyType::ENGINE_TYPE:
            return "engineType";
        case VehiclePropertyType::RSE_EMBEDDED_FIRETVS:
            return "rseEmbeddedFireTvs";
    }

    return "";
}

std::shared_ptr<aace::core::config::EngineConfiguration> VehicleConfiguration::createVehicleInfoConfig(
    const std::vector<VehicleProperty>& propertyList) {
    try {
        json::Value info = {};

        for (auto next : propertyList) {
            std::string name = getVehiclePropertyAttribute(next.first);
            json::Value value = next.second;
            json::set(info, name, value);
        }

        // clang-format off
        json::Value config = {
            {"aace.vehicle",{
                {"info",info}
            }}
        };
        // clang-format on

        return aace::core::config::StreamConfiguration::create(json::toStream(config));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<aace::core::config::EngineConfiguration> VehicleConfiguration::createOperatingCountryConfig(
    const std::string& operatingCountry) {
    try {
        // clang-format off
        json::Value config = {
            {"aace.vehicle",{
                {"operatingCountry",operatingCountry}
            }}
        };
        // clang-format on

        return aace::core::config::StreamConfiguration::create(json::toStream(config));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

}  // namespace config
}  // namespace vehicle
}  // namespace aace
