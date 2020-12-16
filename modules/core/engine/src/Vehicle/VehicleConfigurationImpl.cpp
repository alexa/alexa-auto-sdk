/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace aace {
namespace vehicle {
namespace config {

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
    }

    return "";
}

std::shared_ptr<aace::core::config::EngineConfiguration> VehicleConfiguration::createVehicleInfoConfig(
    const std::vector<VehicleProperty>& propertyList) {
    rapidjson::Document document;

    document.SetObject();

    rapidjson::Value aaceVehicleNode(rapidjson::kObjectType);
    rapidjson::Value infoNode(rapidjson::kObjectType);

    for (auto next : propertyList) {
        std::string name = getVehiclePropertyAttribute(next.first);

        infoNode.AddMember(
            rapidjson::Value().SetString(name.c_str(), name.length(), document.GetAllocator()),
            rapidjson::Value().SetString(next.second.c_str(), next.second.length(), document.GetAllocator()),
            document.GetAllocator());
    }

    aaceVehicleNode.AddMember("info", infoNode, document.GetAllocator());

    document.AddMember("aace.vehicle", aaceVehicleNode, document.GetAllocator());

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    return aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(buffer.GetString()));
}

std::shared_ptr<aace::core::config::EngineConfiguration> VehicleConfiguration::createOperatingCountryConfig(
    const std::string& operatingCountry) {
    rapidjson::Document document;

    document.SetObject();

    rapidjson::Value aaceVehicleNode(rapidjson::kObjectType);

    aaceVehicleNode.AddMember(
        "operatingCountry",
        rapidjson::Value().SetString(operatingCountry.c_str(), operatingCountry.length()),
        document.GetAllocator());

    document.AddMember("aace.vehicle", aaceVehicleNode, document.GetAllocator());

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    return aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(buffer.GetString()));
}

}  // namespace config
}  // namespace vehicle
}  // namespace aace
