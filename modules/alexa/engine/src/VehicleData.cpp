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
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <AACE/Engine/Alexa/VehicleData.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::vehicle;

/// String to identify log entries originating from this file
static const std::string TAG("aace.alexa.VehicleData");

/// VehicleData interface type
static const char VEHICLEDATA_ATTRIBUTE_INTERFACE_TYPE[] = "AlexaInterface";
/// VehicleData interface name
static const char VEHICLEDATA_ATTRIBUTE_INTERFACE_NAME[] = "Alexa.Automotive.VehicleData";
/// VehicleData interface version
static const char VEHICLEDATA_ATTRIBUTE_INTERFACE_VERSION[] = "1.2";
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
/// VehicleData capability attribute: engineType
static const char VEHICLEDATA_ATTRIBUTE_ENGINE_TYPE[] = "engineType";
/// VehicleData capability attribute: vehicleIdentifier
static const char VEHICLEDATA_ATTRIBUTE_VEHICLE_IDENTIFIER[] = "vehicleIdentifier";
/// VehicleData capability attribute: rseEmbeddedFireTvs
static const char VEHICLEDATA_ATTRIBUTE_RSE_EMBEDDED_FIRETVS[] = "rseEmbeddedFireTvs";

std::shared_ptr<VehicleData> VehicleData::create(
    const std::shared_ptr<aace::engine::vehicle::VehicleConfigServiceInterface>& vehicleService) {
    try {
        ThrowIfNull(vehicleService, "Null VehicleConfigServiceInterface");
        auto configuration = getVehicleDataCapabilityConfiguration(vehicleService);
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

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> VehicleData::
    getVehicleDataCapabilityConfiguration(
        const std::shared_ptr<aace::engine::vehicle::VehicleConfigServiceInterface>& vehicleService) {
    try {
        auto additionalConfigurations =
            alexaClientSDK::avsCommon::avs::CapabilityConfiguration::AdditionalConfigurations();

        rapidjson::Document payload(rapidjson::kObjectType);
        auto& allocator = payload.GetAllocator();
        rapidjson::Value resourcesObject(rapidjson::kObjectType);

        auto make = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_MAKE);
        ThrowIf(make.empty(), "missingRequiredMakeAttribute");
        resourcesObject.AddMember(VEHICLEDATA_ATTRIBUTE_MAKE, getTextResourceObject(make, allocator), allocator);

        auto model = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_MODEL);
        ThrowIf(model.empty(), "missingRequiredModelAttribute");
        resourcesObject.AddMember(VEHICLEDATA_ATTRIBUTE_MODEL, getTextResourceObject(model, allocator), allocator);

        auto trim = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_TRIM);
        if (!trim.empty()) {
            resourcesObject.AddMember(VEHICLEDATA_ATTRIBUTE_TRIM, getTextResourceObject(trim, allocator), allocator);
        }

        payload.AddMember("resources", resourcesObject, allocator);

        auto year = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_YEAR);
        ThrowIf(year.empty(), "yearAttributeIsEmpty");
        payload.AddMember(VEHICLEDATA_ATTRIBUTE_YEAR, (int64_t)std::stol(year), allocator);

        auto engineType = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_ENGINE_TYPE);
        if (!engineType.empty()) {
            payload.AddMember(VEHICLEDATA_ATTRIBUTE_ENGINE_TYPE, engineType, allocator);
        } else {
            AACE_DEBUG(LX(TAG, "skippingEmptyEngineType"));
        }

        auto vehicleIdentifier = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_VEHICLE_ID);
        if (!vehicleIdentifier.empty()) {
            payload.AddMember(VEHICLEDATA_ATTRIBUTE_VEHICLE_IDENTIFIER, vehicleIdentifier, allocator);
        } else {
            AACE_DEBUG(LX(TAG, "skippingEmptyVehicleIdentifier"));
        }

        auto rseEmbeddedFireTvs = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_RSE_FIRE_TVS);
        if (!rseEmbeddedFireTvs.empty()) {
            payload.AddMember(
                VEHICLEDATA_ATTRIBUTE_RSE_EMBEDDED_FIRETVS, (int64_t)std::stol(rseEmbeddedFireTvs), allocator);
        } else {
            AACE_DEBUG(LX(TAG, "skippingEmptyRseEmbeddedFireTvs"));
        }

        bool includeAnalyticsSegments = false;
        rapidjson::Value analyticsObject(rapidjson::kObjectType);
        rapidjson::Value analyticsSegmentsArray(rapidjson::kArrayType);

        auto osVersion = vehicleService->getDeviceInfoValue(KEY_DEVICE_INFO_OS_VERSION);
        auto platform = vehicleService->getDeviceInfoValue(KEY_DEVICE_INFO_PLATFORM);
        std::string os;
        if (!platform.empty()) {
            os = !osVersion.empty() ? platform + " " + osVersion : platform;
        }
        if (!os.empty()) {
            analyticsSegmentsArray.PushBack(getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_OS, os, allocator), allocator);
            includeAnalyticsSegments = true;
        }

        auto arch = vehicleService->getDeviceInfoValue(KEY_DEVICE_INFO_HARDWARE_ARCH);
        if (!arch.empty()) {
            analyticsSegmentsArray.PushBack(getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_ARCH, arch, allocator), allocator);
            includeAnalyticsSegments = true;
        }

        auto microphone = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_MIC_TYPE);
        if (!microphone.empty()) {
            analyticsSegmentsArray.PushBack(
                getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_MIC, microphone, allocator), allocator);
            includeAnalyticsSegments = true;
        }

        auto geography = vehicleService->getVehicleInfoValue(KEY_VEHICLE_INFO_OPERATING_COUNTRY);
        if (!geography.empty()) {
            analyticsSegmentsArray.PushBack(
                getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_GEOGRAPHY, geography, allocator), allocator);
            includeAnalyticsSegments = true;
        }

        auto version = vehicleService->getAppInfoValue(KEY_APP_INFO_SOFTWARE_VERSION);
        if (!version.empty()) {
            analyticsSegmentsArray.PushBack(
                getAnalyticsObject(VEHICLEDATA_ATTRIBUTE_VERSION, version, allocator), allocator);
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
