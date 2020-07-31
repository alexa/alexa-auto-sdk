/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Alexa/VehicleData.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <string>

namespace aace {
namespace engine {
namespace alexa {

/// String to identify log entries originating from this file
static const std::string TAG("aace.alexa.VehicleData");

/// VehicleData interface type
static const char VEHICLEDATA_ATTRIBUTE_INTERFACE_TYPE[] = "AlexaInterface";
/// VehicleData interface name
static const char VEHICLEDATA_ATTRIBUTE_INTERFACE_NAME[] = "Alexa.Automotive.VehicleData";
/// VehicleData interface version
static const char VEHICLEDATA_ATTRIBUTE_INTERFACE_VERSION[] = "1.0";
/// VehicleData capability analytics attribute: operating system
static const char VEHICLEDATA_ATTRIBUTE_OS[] = "OPERATING_SYSTEM";
/// VehicleData capability analytics attribute: hardware architecture
static const char VEHICLEDATA_ATTRIBUTE_ARCH[] = "HARDWARE_ARCHITECTURE";
/// VehicleData capability analytics attribute: microphone type
static const char VEHICLEDATA_ATTRIBUTE_MIC[] = "MICROPHONE_TYPE";
/// VehicleData capability analytics attribute: geography
static const char VEHICLEDATA_ATTRIBUTE_GEOGRAPHY[] = "GEOGRAPHY";
/// VehicleData capability analytics attribute: application version
static const char VEHICLEDATA_ATTRIBUTE_VERSION[] = "ALEXA_APPLICATION_VERSION";
/// VehicleData capability resources attribute: make
static const char VEHICLEDATA_ATTRIBUTE_MAKE[] = "make";
/// VehicleData capability resources attribute: model
static const char VEHICLEDATA_ATTRIBUTE_MODEL[] = "model";
/// VehicleData capability resources attribute: trim
static const char VEHICLEDATA_ATTRIBUTE_TRIM[] = "trim";
/// VehicleData capability attribute: year
static const char VEHICLEDATA_ATTRIBUTE_YEAR[] = "year";

/// Map from a capability attribute string to its corresponding @c VehiclePropertyType
static std::unordered_map<std::string, VehicleData::VehiclePropertyType> s_attributeToVehiclePropertyMap = {
    {VEHICLEDATA_ATTRIBUTE_OS, VehicleData::VehiclePropertyType::OPERATING_SYSTEM},
    {VEHICLEDATA_ATTRIBUTE_ARCH, VehicleData::VehiclePropertyType::HARDWARE_ARCH},
    {VEHICLEDATA_ATTRIBUTE_MIC, VehicleData::VehiclePropertyType::MICROPHONE},
    {VEHICLEDATA_ATTRIBUTE_GEOGRAPHY, VehicleData::VehiclePropertyType::GEOGRAPHY},
    {VEHICLEDATA_ATTRIBUTE_VERSION, VehicleData::VehiclePropertyType::VERSION},
    {VEHICLEDATA_ATTRIBUTE_MAKE, VehicleData::VehiclePropertyType::MAKE},
    {VEHICLEDATA_ATTRIBUTE_MODEL, VehicleData::VehiclePropertyType::MODEL},
    {VEHICLEDATA_ATTRIBUTE_TRIM, VehicleData::VehiclePropertyType::TRIM},
    {VEHICLEDATA_ATTRIBUTE_YEAR, VehicleData::VehiclePropertyType::YEAR}};

std::shared_ptr<VehicleData> VehicleData::create(const VehiclePropertyMap& vehiclePropertyMap) {
    try {
        // Note: the create() method creates the CapabilityConfiguration at this point to validate vehiclePropertyMap
        auto configuration = getVehicleDataCapabilityConfiguration(vehiclePropertyMap);
        ThrowIfNull(configuration, "couldNotCreateCapabilityConfiguration");
        std::shared_ptr<VehicleData> vehicleData = std::shared_ptr<VehicleData>(new VehicleData(configuration));
        return vehicleData;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

VehicleData::VehicleData(std::shared_ptr<CapabilityConfiguration> capabilityConfig) {
    m_capabilityConfigurations.insert(capabilityConfig);
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> VehicleData::
    getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

alexaClientSDK::avsCommon::utils::Optional<std::string> VehicleData::getPropertyByAttribute(
    const std::string& attribute,
    const VehiclePropertyMap& vehiclePropertyMap) {
    alexaClientSDK::avsCommon::utils::Optional<std::string> value;
    auto propertyType = s_attributeToVehiclePropertyMap[attribute];
    auto it = vehiclePropertyMap.find(propertyType);
    if (it != vehiclePropertyMap.end()) {
        value.set(it->second);
    }
    return value;
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> VehicleData::
    getVehicleDataCapabilityConfiguration(const VehiclePropertyMap& vehiclePropertyMap) {
    try {
        auto additionalConfigurations =
            alexaClientSDK::avsCommon::avs::CapabilityConfiguration::AdditionalConfigurations();

        rapidjson::Document payload(rapidjson::kObjectType);
        auto& allocator = payload.GetAllocator();
        rapidjson::Value resourcesObject(rapidjson::kObjectType);

        auto make = getPropertyByAttribute(VEHICLEDATA_ATTRIBUTE_MAKE, vehiclePropertyMap);
        ThrowIfNot(make.hasValue(), "missingRequiredMakeAttribute");
        resourcesObject.AddMember(
            VEHICLEDATA_ATTRIBUTE_MAKE, getTextResourceObject(make.value(), allocator), allocator);

        auto model = getPropertyByAttribute(VEHICLEDATA_ATTRIBUTE_MODEL, vehiclePropertyMap);
        ThrowIfNot(model.hasValue(), "missingRequiredModelAttribute");
        resourcesObject.AddMember(
            VEHICLEDATA_ATTRIBUTE_MODEL, getTextResourceObject(model.value(), allocator), allocator);

        auto trim = getPropertyByAttribute(VEHICLEDATA_ATTRIBUTE_TRIM, vehiclePropertyMap);
        if (trim.hasValue()) {
            resourcesObject.AddMember(
                VEHICLEDATA_ATTRIBUTE_TRIM, getTextResourceObject(trim.value(), allocator), allocator);
        }

        payload.AddMember("resources", resourcesObject, allocator);

        auto year = getPropertyByAttribute(VEHICLEDATA_ATTRIBUTE_YEAR, vehiclePropertyMap);
        ThrowIfNot(year.hasValue(), "missingRequiredYearAttribute");
        ThrowIf(year.value() == "", "yearAttributeIsEmpty");
        payload.AddMember(VEHICLEDATA_ATTRIBUTE_YEAR, (int64_t)std::stol(year.value()), allocator);

        bool includeAnalyticsSegments = false;
        rapidjson::Value analyticsObject(rapidjson::kObjectType);
        rapidjson::Value analyticsSegmentsArray(rapidjson::kArrayType);

        auto os = getPropertyByAttribute(VEHICLEDATA_ATTRIBUTE_OS, vehiclePropertyMap);
        if (os.hasValue()) {
            analyticsSegmentsArray.PushBack(
                getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_OS, os.value(), allocator), allocator);
            includeAnalyticsSegments = true;
        }

        auto arch = getPropertyByAttribute(VEHICLEDATA_ATTRIBUTE_ARCH, vehiclePropertyMap);
        if (arch.hasValue()) {
            analyticsSegmentsArray.PushBack(
                getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_ARCH, arch.value(), allocator), allocator);
            includeAnalyticsSegments = true;
        }

        auto microphone = getPropertyByAttribute(VEHICLEDATA_ATTRIBUTE_MIC, vehiclePropertyMap);
        if (microphone.hasValue()) {
            analyticsSegmentsArray.PushBack(
                getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_MIC, microphone.value(), allocator), allocator);
            includeAnalyticsSegments = true;
        }

        auto geography = getPropertyByAttribute(VEHICLEDATA_ATTRIBUTE_GEOGRAPHY, vehiclePropertyMap);
        if (geography.hasValue()) {
            analyticsSegmentsArray.PushBack(
                getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_GEOGRAPHY, geography.value(), allocator), allocator);
            includeAnalyticsSegments = true;
        }

        auto version = getPropertyByAttribute(VEHICLEDATA_ATTRIBUTE_VERSION, vehiclePropertyMap);
        if (version.hasValue()) {
            analyticsSegmentsArray.PushBack(
                getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_VERSION, version.value(), allocator), allocator);
            includeAnalyticsSegments = true;
        }

        if (includeAnalyticsSegments) {
            analyticsObject.AddMember("segments", analyticsSegmentsArray, allocator);
            payload.AddMember("analytics", analyticsObject, allocator);
        } else {
            AACE_DEBUG(LX(TAG, "skippingEmptyAnalyticsSegments"));
        }

        // Finish the RapidJSON doc
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        ThrowIfNot(payload.Accept(writer), "writerRefusedJsonObject");

        additionalConfigurations.insert({"configuration", buffer.GetString()});
        return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(
            VEHICLEDATA_ATTRIBUTE_INTERFACE_TYPE,
            VEHICLEDATA_ATTRIBUTE_INTERFACE_NAME,
            VEHICLEDATA_ATTRIBUTE_INTERFACE_VERSION,
            alexaClientSDK::avsCommon::utils::Optional<std::string>(),
            alexaClientSDK::avsCommon::utils::Optional<
                alexaClientSDK::avsCommon::avs::CapabilityConfiguration::Properties>(),
            additionalConfigurations);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "generateCapabilityConfigurationFailed").d("reason", ex.what()));
        return nullptr;
    }
}

rapidjson::Value VehicleData::getAnalyticsObject(
    const std::string& key,
    const std::string& value,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value analyticsObject(rapidjson::kObjectType);
    analyticsObject.AddMember("key", key, allocator);
    analyticsObject.AddMember("value", value, allocator);
    return analyticsObject;
}

rapidjson::Value VehicleData::getTextResourceObject(
    const std::string& value,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value resourceObject(rapidjson::kObjectType);
    resourceObject.AddMember("@type", "text", allocator);

    rapidjson::Value valueObject(rapidjson::kObjectType);
    valueObject.AddMember("text", value, allocator);
    // note: en-US is the only supported locale
    valueObject.AddMember("locale", "en-US", allocator);

    resourceObject.AddMember("value", valueObject, allocator);
    return resourceObject;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
