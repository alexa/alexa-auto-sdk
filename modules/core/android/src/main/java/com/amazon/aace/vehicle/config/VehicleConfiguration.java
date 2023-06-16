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

package com.amazon.aace.vehicle.config;

import com.amazon.aace.core.config.EngineConfiguration;

/**
 * The @c VehicleConfiguration class is a factory that creates "aace.vehicle"
 * Engine configuration objects.
 */
public class VehicleConfiguration {
    /**
     * Specifies the vehicle info configuration keys
     */
    public enum VehicleInfoPropertyType {
        /**
         * The make of the vehicle.
         * @note This property is required.
         *
         * @hideinitializer
         */
        MAKE("MAKE", "make"),

        /**
         * The model of the vehicle.
         * @note This property is required.
         *
         * @hideinitializer
         */
        MODEL("MODEL", "model"),

        /**
         * The model year of the vehicle.
         * A value of this property type must be an integer in the range
         * 1900-2100 expressed as a string.
         * Example value: "2019"
         * @note This property is required.
         *
         * @hideinitializer
         */
        YEAR("YEAR", "year"),

        /**
         * The trim package of the vehicle, identifying the vehicle's level of
         * equipment or special features.
         * Example values: "Standard", "Sport", "Limited"
         *
         * @hideinitializer
         */
        TRIM("TRIM", "trim"),

        /**
         * The current (or intended, if current is not available) operating
         * country for the vehicle. The value must be an ISO 3166
         * Alpha-2 country code.
         * Example values: "US", "MX", "JP"
         *
         * @hideinitializer
         */
        OPERATING_COUNTRY("OPERATING_COUNTRY", "operatingCountry"),

        /**
         * The type and arrangement of microphone used by the vehicle.
         * Example value: "7 mic array centrally mounted"
         *
         * @hideinitializer
         */
        MICROPHONE_TYPE("MICROPHONE_TYPE", "microphoneType"),

        /**
         * The automaker's identifier for the vehicle.
         *
         * @hideinitializer
         */
        VEHICLE_IDENTIFIER("VEHICLE_IDENTIFIER", "vehicleIdentifier"),

        /**
         * The type of engine in the vehicle.
         * Accepted values: "GAS", "HYBRID", "ELECTRIC"
         *
         * @hideinitializer
         */
        ENGINE_TYPE("ENGINE_TYPE", "engineType"),

        /**
         * The number of rear seat embedded Fire TVs in the vehicle. The value
         * must be an integer expressed as a string.
         * Example value: "2"
         *
         * @hideinitializer
         */
        RSE_EMBEDDED_FIRE_TVS("RSE_EMBEDDED_FIRE_TVS", "rseEmbeddedFireTvs");

        /**
         * @internal
         */
        private String mName;

        /**
         * @internal
         */
        private String mKey;

        /**
         * @internal
         */
        VehicleInfoPropertyType(String name, String key) {
            mName = name;
            mKey = key;
        }

        /**
         * @internal
         */
        public String toString() {
            return mName;
        }

        /**
         * @internal
         */
        public String getKey() {
            return mKey;
        }
    }

    /**
     * Identifies a vehicle info property with a pair of type and value.
     */
    public static class VehicleInfoProperty {
        private VehicleInfoPropertyType mType;
        private String mValue;

        public VehicleInfoProperty(VehicleInfoPropertyType type, String value) {
            mType = type;
            mValue = value;
        }

        public VehicleInfoPropertyType getType() {
            return mType;
        }
        public String getValue() {
            return mValue;
        }
    }

    /**
     * Specifies the device info configuration keys
     */
    public enum DeviceInfoPropertyType {
        /**
         * The manufacturer of the head unit hardware.
         * Example values: "Alpine", "Pioneer"
         *
         * @hideinitializer
         */
        MANUFACTURER("MANUFACTURER", "manufacturer"),

        /**
         * The model name of the head unit hardware.
         * Example value: "Coral"
         *
         * @hideinitializer
         */
        MODEL("MODEL", "model"),

        /**
         * The serial number of the head unit expressed as a string.
         *
         * @hideinitializer
         */
        SERIAL_NUMBER("SERIAL_NUMBER", "serialNumber"),

        /**
         * The head unit software platform or operating system name.
         * Example values: "Android", "Ubuntu"
         *
         * @hideinitializer
         */
        PLATFORM("PLATFORM", "platform"),

        /**
         * The version of the head unit operating system expressed as a string.
         * Example values: "12", "18.04.6 LTS"
         *
         * @hideinitializer
         */
        OS_VERSION("OS_VERSION", "osVersion"),

        /**
         * The hardware architecture of the head unit or CPU+instruction set.
         * Examples: "arm64-v8a", "x86_64", "armv7hf", "armv8"
         *
         * @hideinitializer
         */
        HARDWARE_ARCH("HARDWARE_ARCH", "hardwareArch");

        /**
         * @internal
         */
        private String mName;

        /**
         * @internal
         */
        private String mKey;

        /**
         * @internal
         */
        DeviceInfoPropertyType(String name, String key) {
            mName = name;
            mKey = key;
        }

        /**
         * @internal
         */
        public String toString() {
            return mName;
        }

        /**
         * @internal
         */
        public String getKey() {
            return mKey;
        }
    }

    /**
     * Identifies a device info property with a pair of type and value.
     */
    public static class DeviceInfoProperty {
        private DeviceInfoPropertyType mType;
        private String mValue;

        public DeviceInfoProperty(DeviceInfoPropertyType type, String value) {
            mType = type;
            mValue = value;
        }

        public DeviceInfoPropertyType getType() {
            return mType;
        }
        public String getValue() {
            return mValue;
        }
    }

    /**
     * Specifies the app info configuration keys
     */
    public enum AppInfoPropertyType {
        /**
         * The version of the Auto SDK client application expressed as a string.
         * Example value: "1.0.1"
         *
         * @hideinitializer
         */
        SOFTWARE_VERSION("SOFTWARE_VERSION", "softwareVersion");

        /**
         * @internal
         */
        private String mName;

        /**
         * @internal
         */
        private String mKey;

        /**
         * @internal
         */
        AppInfoPropertyType(String name, String key) {
            mName = name;
            mKey = key;
        }

        /**
         * @internal
         */
        public String toString() {
            return mName;
        }

        /**
         * @internal
         */
        public String getKey() {
            return mKey;
        }
    }

    /**
     * Identifies an app info property with a pair of type and value.
     */
    public static class AppInfoProperty {
        private AppInfoPropertyType mType;
        private String mValue;

        public AppInfoProperty(AppInfoPropertyType type, String value) {
            mType = type;
            mValue = value;
        }

        public AppInfoPropertyType getType() {
            return mType;
        }
        public String getValue() {
            return mValue;
        }
    }

    /**
     * Factory method used to programmatically generate
     * "aace.vehicle.vehicleInfo" Engine configuration data. The data
     * generated by this method is equivalent to providing the following JSON
     * format in a configuration file:
     *
     * @code{.json}
     * {
     *   "aace.vehicle": {
     *      "vehicleInfo": {
     *          "make": "<VEHICLE_MAKE>",
     *          "model": "<VEHICLE_MODEL>",
     *          "year": "<VEHICLE_MODEL_YEAR>",
     *          "trim": "<TRIM>",
     *          "microphoneType": "<MICROPHONE_TYPE>",
     *          "operatingCountry": "<OPERATING_COUNTRY>",
     *          "vehicleIdentifier": "<VEHICLE_ID>",
     *          "engineType": "<ENGINE_TYPE>",
     *          "rseEmbeddedFireTvs": "<EMBEDDED_FIRE_TV_COUNT>"
     *      }
     *   }
     * }
     * @endcode
     *
     * @param propertyList A list of @c VehicleInfoProperty pairs
     * @return A @c com.amazon.aace.core.config.EngineConfiguration containing
     *         the specified data in JSON format
     */
    public static EngineConfiguration createVehicleInfoConfig(final VehicleInfoProperty[] propertyList) {
        return new EngineConfiguration() {
            @Override
            protected long createNativeRef() {
                return createVehicleInfoConfigBinder(propertyList);
            }
        };
    }

    /**
     * Factory method used to programmatically generate
     * "aace.vehicle.deviceInfo" Engine configuration data. The data
     * generated by this method is equivalent to providing the following JSON
     * format in a configuration file:
     *
     * @code{.json}
     * {
     *   "aace.vehicle": {
     *      "deviceInfo": {
     *          "manufacturer": "<DEVICE_MANUFACTURER>",
     *          "model": "<DEVICE_MODEL>",
     *          "serialNumber": <DEVICE_SERIAL_NUMBER>,
     *          "osVersion": "<OS_VERSION>",
     *          "hardwareArch": "<HARDWARE_ARCH>",
     *          "platform": "<PLATFORM>",
     *      }
     *   }
     * }
     * @endcode
     *
     * @param propertyList A list of @c DeviceInfoProperty pairs
     * @return A @c com.amazon.aace.core.config.EngineConfiguration containing
     *         the specified data in JSON format
     */
    public static EngineConfiguration createDeviceInfoConfig(final DeviceInfoProperty[] propertyList) {
        return new EngineConfiguration() {
            @Override
            protected long createNativeRef() {
                return createDeviceInfoConfigBinder(propertyList);
            }
        };
    }

    /**
     * Factory method used to programmatically generate
     * "aace.vehicle.appInfo" Engine configuration data. The data
     * generated by this method is equivalent to providing the following JSON
     * format in a configuration file:
     *
     * @code{.json}
     * {
     *   "aace.vehicle": {
     *      "appInfo": {
     *          "softwareVersion": "<APP_VERSION>"
     *      }
     *   }
     * }
     * @endcode
     *
     * @param propertyList A list of @c AppInfoProperty pairs
     * @return A @c com.amazon.aace.core.config.EngineConfiguration containing
     *         the specified data in JSON format
     */
    public static EngineConfiguration createAppInfoConfig(final AppInfoProperty[] propertyList) {
        return new EngineConfiguration() {
            @Override
            protected long createNativeRef() {
                return createAppInfoConfigBinder(propertyList);
            }
        };
    }

    // Native Engine JNI methods
    static private native long createVehicleInfoConfigBinder(VehicleInfoProperty[] propertyList);
    static private native long createDeviceInfoConfigBinder(DeviceInfoProperty[] propertyList);
    static private native long createAppInfoConfigBinder(AppInfoProperty[] propertyList);
}
