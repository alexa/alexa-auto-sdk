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

#ifndef AACE_ENGINE_VEHICLE_VEHICLE_PROPERTY_INTERFACE_H
#define AACE_ENGINE_VEHICLE_VEHICLE_PROPERTY_INTERFACE_H

#include <string>

namespace aace {
namespace engine {
namespace vehicle {

const std::string KEY_VEHICLE_INFO_MAKE = "make";
const std::string KEY_VEHICLE_INFO_MODEL = "model";
const std::string KEY_VEHICLE_INFO_YEAR = "year";
const std::string KEY_VEHICLE_INFO_TRIM = "trim";
const std::string KEY_VEHICLE_INFO_MIC_TYPE = "microphoneType";
const std::string KEY_VEHICLE_INFO_OPERATING_COUNTRY = "operatingCountry";
const std::string KEY_VEHICLE_INFO_VEHICLE_ID = "vehicleIdentifier";
const std::string KEY_VEHICLE_INFO_ENGINE_TYPE = "engineType";
const std::string KEY_VEHICLE_INFO_RSE_FIRE_TVS = "rseEmbeddedFireTvs";

const std::string KEY_APP_INFO_SOFTWARE_VERSION = "softwareVersion";

const std::string KEY_DEVICE_INFO_MANUFACTURER = "manufacturer";
const std::string KEY_DEVICE_INFO_MODEL = "model";
const std::string KEY_DEVICE_INFO_OS_VERSION = "osVersion";
const std::string KEY_DEVICE_INFO_HARDWARE_ARCH = "hardwareArch";
const std::string KEY_DEVICE_INFO_PLATFORM = "platform";
const std::string KEY_DEVICE_INFO_SERIAL_NUMBER = "serialNumber";

/**
 * The @c VehicleConfigServiceInterface shares vehicle configuration settings
 */
class VehicleConfigServiceInterface {
public:
    /// Destructor
    virtual ~VehicleConfigServiceInterface() = default;

    /**
     * Retrieves the value of the specified key configured in
     * @c "aace.vehicle.vehicleInfo".
     *
     * @param key The key corresponding to the value to retrieve
     * @return The value corresponding to the specified key or an empty string
     *         if there is no value configured.
     * @throw @c std::invalid_argument exception if the specified string is not
     *        a valid key
     */
    virtual std::string getVehicleInfoValue(const std::string& key) = 0;

    /**
     * Retrieves the value of the specified key configured in
     * @c "aace.vehicle.appInfo".
     *
     * @param key The key corresponding to the value to retrieve
     * @return The value corresponding to the specified key or an empty string
     *         if there is no value configured.
     * @throw @c std::invalid_argument exception if the specified string is not
     *        a valid key
     */
    virtual std::string getAppInfoValue(const std::string& key) = 0;

    /**
     * Retrieves the value of the specified key configured in
     * @c "aace.vehicle.deviceInfo".
     *
     * @param key The key corresponding to the value to retrieve
     * @return The value corresponding to the specified key or an empty string
     *         if there is no value configured.
     * @throw @c std::invalid_argument exception if the specified string is not
     *        a valid key
     */
    virtual std::string getDeviceInfoValue(const std::string& key) = 0;
};

}  // namespace vehicle
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_VEHICLE_VEHICLE_PROPERTY_INTERFACE_H
