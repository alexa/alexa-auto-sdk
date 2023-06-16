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

#ifndef AACE_ENGINE_METRICS_METRICS_CONFIG_INTERFACE_H
#define AACE_ENGINE_METRICS_METRICS_CONFIG_INTERFACE_H

#include <string>
#include <unordered_map>

namespace aace {
namespace engine {
namespace metrics {

typedef std::unordered_map<std::string, std::string> DimensionsMap;

const std::string PREDEFINED_KEY_BUILD_TYPE = "_buildType";
const std::string PREDEFINED_KEY_PLATFORM = "_platform";
const std::string PREDEFINED_KEY_HARDWARE = "_hardware";
const std::string PREDEFINED_KEY_MODEL = "_model";
const std::string PREDEFINED_KEY_SOFTWARE_VERSION = "_softwareVersion";
const std::string PREDEFINED_KEY_DEVICE_LANGUAGE = "_deviceLanguage";
const std::string PREDEFINED_KEY_TIME_ZONE = "_timeZone";

const std::string CUSTOM_KEY_AUTO_SDK_VERSION = "AutoSdkVersion";
const std::string CUSTOM_KEY_VEHICLE_MAKE = "VehicleMake";
const std::string CUSTOM_KEY_VEHICLE_MODEL = "VehicleModel";
const std::string CUSTOM_KEY_VEHICLE_YEAR = "VehicleYear";
const std::string CUSTOM_KEY_OPERATING_COUNTRY = "OperatingCountry";

const std::string CUSTOM_KEY_UNIQUE_ANON_DEVICE_ID = "UniqueAnonDeviceId";
const std::string CUSTOM_KEY_AGENT_ID = "AgentId";

const std::string DIMENSION_VALUE_UNKNOWN = "UNKNOWN";
const std::string DIMENSION_VALUE_BUILD_TYPE_RELEASE = "Release";
const std::string DIMENSION_VALUE_BUILD_TYPE_DEBUG = "Debug";
const std::string DIMENSION_VALUE_BUILD_TYPE_QA = "QA";

const DimensionsMap g_predefinedDimensionDefaults = {{PREDEFINED_KEY_BUILD_TYPE, DIMENSION_VALUE_BUILD_TYPE_RELEASE},
                                                     {PREDEFINED_KEY_PLATFORM, DIMENSION_VALUE_UNKNOWN},
                                                     {PREDEFINED_KEY_HARDWARE, DIMENSION_VALUE_UNKNOWN},
                                                     {PREDEFINED_KEY_MODEL, DIMENSION_VALUE_UNKNOWN},
                                                     {PREDEFINED_KEY_SOFTWARE_VERSION, DIMENSION_VALUE_UNKNOWN},
                                                     {PREDEFINED_KEY_DEVICE_LANGUAGE, DIMENSION_VALUE_UNKNOWN},
                                                     {PREDEFINED_KEY_TIME_ZONE, DIMENSION_VALUE_UNKNOWN}};

const DimensionsMap g_customDimensionDefaults = {{CUSTOM_KEY_AUTO_SDK_VERSION, DIMENSION_VALUE_UNKNOWN},
                                                 {CUSTOM_KEY_VEHICLE_MAKE, DIMENSION_VALUE_UNKNOWN},
                                                 {CUSTOM_KEY_VEHICLE_MODEL, DIMENSION_VALUE_UNKNOWN},
                                                 {CUSTOM_KEY_VEHICLE_YEAR, DIMENSION_VALUE_UNKNOWN},
                                                 {CUSTOM_KEY_OPERATING_COUNTRY, DIMENSION_VALUE_UNKNOWN}};

/**
 * The @c MetricsConfigServiceInterface shares configuration settings of the
 * metrics service.
 */
class MetricsConfigServiceInterface {
public:
    /**
     * Destructor
     */
    virtual ~MetricsConfigServiceInterface() = default;

    /**
     * Gets the absolute path on the device where metrics can be stored before
     * upload.
     * 
     * @return The metric storage path
     */
    virtual std::string getMetricStoragePath() = 0;

    /**
     * Gets the stable, unique anonymous identifier for the device.
     * 
     * @return A pair mapping the key the uploader should use for the ID to the
     *         ID value
     */
    virtual std::pair<std::string, std::string> getStableUniqueAnonymousId() = 0;

    /**
     * Gets the values of the uploader client's "predefined" keys that the 
     * uploader should insert as dimensions into every metric. The uploader
     * might have its own key for any particular value.
     * 
     * @return A map of predefined common dimension keys to their values
     */
    virtual DimensionsMap getPredefinedCommonDimensions() = 0;

    /**
     * Gets the values of Auto SDK's own "custom" keys that the uploader
     * implementation should insert as dimensions into every metric.
     * The uploader must use the key names exactly as they appear in the map.
     * 
     * @return A map of Auto SDK's custom common dimension keys to their values
     */
    virtual DimensionsMap getCustomCommonDimensions() = 0;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRICS_CONFIG_INTERFACE_H
