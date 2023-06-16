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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Vehicle/VehicleConfiguration.h>

namespace json = aace::engine::utils::json;
using VehicleInfoPropertyType = aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType;
using DeviceInfoPropertyType = aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType;
using AppInfoPropertyType = aace::vehicle::config::VehicleConfiguration::AppInfoPropertyType;

/// Test harness for @c VehicleConfigurationImpl class
class VehicleConfigurationImplTest : public ::testing::Test {
public:
    void SetUp() override {
        /* Empty Setup() for keeping TEST_F uniformity */
    }
};

/**
 * Test createVehicleInfoConfig() expecting a valid VehicleConfigurationImplTest to be returned.
 */
TEST_F(VehicleConfigurationImplTest, createValidVehicleInfoConfigWithAllValues) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"vehicleInfo",{
                {"make","Honda"},
                {"model","Odyssey"},
                {"trim","EX-L"},
                {"year","2019"},
                {"operatingCountry","US"},
                {"microphoneType","SingleArray"},
                {"vehicleIdentifier","123456789a"},
                {"engineType","GAS"},
                {"rseEmbeddedFireTvs","1"}
            }}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createVehicleInfoConfig(
        {{VehicleInfoPropertyType::MAKE, "Honda"},
         {VehicleInfoPropertyType::MODEL, "Odyssey"},
         {VehicleInfoPropertyType::TRIM, "EX-L"},
         {VehicleInfoPropertyType::YEAR, "2019"},
         {VehicleInfoPropertyType::OPERATING_COUNTRY, "US"},
         {VehicleInfoPropertyType::MICROPHONE_TYPE, "SingleArray"},
         {VehicleInfoPropertyType::VEHICLE_IDENTIFIER, "123456789a"},
         {VehicleInfoPropertyType::ENGINE_TYPE, "GAS"},
         {VehicleInfoPropertyType::RSE_EMBEDDED_FIRE_TVS, "1"}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

/**
 * Test create() expecting a valid VehicleConfigurationImplTest to be returned.
 */
TEST_F(VehicleConfigurationImplTest, createValidVehicleInfoConfigWithSomeEmptyValues) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"vehicleInfo",{
                {"make","Honda"},
                {"model","Odyssey"},
                {"trim","EX-L"},
                {"year","2019"},
                {"operatingCountry","US"},
                {"microphoneType",""},
                {"vehicleIdentifier","123456789a"},
                {"engineType","GAS"},
                {"rseEmbeddedFireTvs","1"}
            }}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createVehicleInfoConfig(
        {{VehicleInfoPropertyType::MAKE, "Honda"},
         {VehicleInfoPropertyType::MODEL, "Odyssey"},
         {VehicleInfoPropertyType::TRIM, "EX-L"},
         {VehicleInfoPropertyType::YEAR, "2019"},
         {VehicleInfoPropertyType::OPERATING_COUNTRY, "US"},
         {VehicleInfoPropertyType::MICROPHONE_TYPE, ""},
         {VehicleInfoPropertyType::VEHICLE_IDENTIFIER, "123456789a"},
         {VehicleInfoPropertyType::ENGINE_TYPE, "GAS"},
         {VehicleInfoPropertyType::RSE_EMBEDDED_FIRE_TVS, "1"}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

/**
 * Test create() expecting a valid VehicleConfigurationImplTest to be returned.
 */
TEST_F(VehicleConfigurationImplTest, createValidVehicleInfoConfigWithSomeMissingValues) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"vehicleInfo",{
                {"make","Honda"},
                {"model","Odyssey"},
                {"trim","EX-L"},
                {"year","2019"},
                {"microphoneType",""},
                {"vehicleIdentifier","123456789a"}
            }}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createVehicleInfoConfig(
        {{VehicleInfoPropertyType::MAKE, "Honda"},
         {VehicleInfoPropertyType::MODEL, "Odyssey"},
         {VehicleInfoPropertyType::TRIM, "EX-L"},
         {VehicleInfoPropertyType::YEAR, "2019"},
         {VehicleInfoPropertyType::MICROPHONE_TYPE, ""},
         {VehicleInfoPropertyType::VEHICLE_IDENTIFIER, "123456789a"}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

TEST_F(VehicleConfigurationImplTest, createValidDeviceInfoConfigWithAllValues) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"deviceInfo",{
                {"manufacturer","Apple"},
                {"model","MacBook Pro"},
                {"serialNumber","123qwe456tyu"},
                {"osVersion","13.2.1"},
                {"hardwareArch","x86_64"},
                {"platform","macOS"}
            }}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createDeviceInfoConfig(
        {{DeviceInfoPropertyType::MANUFACTURER, "Apple"},
         {DeviceInfoPropertyType::MODEL, "MacBook Pro"},
         {DeviceInfoPropertyType::SERIAL_NUMBER, "123qwe456tyu"},
         {DeviceInfoPropertyType::OS_VERSION, "13.2.1"},
         {DeviceInfoPropertyType::HARDWARE_ARCH, "x86_64"},
         {DeviceInfoPropertyType::PLATFORM, "macOS"}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

TEST_F(VehicleConfigurationImplTest, createValidDeviceInfoConfigWithSomeMissingValues) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"deviceInfo",{
                {"manufacturer","Apple"},
                {"serialNumber","123qwe456tyu"},
                {"hardwareArch","x86_64"},
                {"platform","macOS"}
            }}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createDeviceInfoConfig(
        {{DeviceInfoPropertyType::MANUFACTURER, "Apple"},
         {DeviceInfoPropertyType::SERIAL_NUMBER, "123qwe456tyu"},
         {DeviceInfoPropertyType::HARDWARE_ARCH, "x86_64"},
         {DeviceInfoPropertyType::PLATFORM, "macOS"}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

TEST_F(VehicleConfigurationImplTest, createValidAppInfoConfigWithAllValues) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"appInfo",{
                {"softwareVersion","4.2"}
            }}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createAppInfoConfig(
        {{AppInfoPropertyType::SOFTWARE_VERSION, "4.2"}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

TEST_F(VehicleConfigurationImplTest, createValidAppInfoConfigWithEmptyValues) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"appInfo",{
                {"softwareVersion",""}
            }}
        }}
    };
    // clang-format on

    testConfiguration =
        aace::vehicle::config::VehicleConfiguration::createAppInfoConfig({{AppInfoPropertyType::SOFTWARE_VERSION, ""}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}
