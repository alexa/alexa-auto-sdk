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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

#include <AACE/Engine/Utils/JSON/JSON.h>
#include <AACE/Vehicle/VehicleConfiguration.h>

// json namespace alias
namespace json = aace::engine::utils::json;

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
TEST_F(VehicleConfigurationImplTest, createVehicleInfoConfig) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"info",{
                {"make","Honda"},
                {"model","Odyssey"},
                {"trim","EX-L"},
                {"year","2019"},
                {"geography","US"},
                {"version","1.0"},
                {"os","darwin"},
                {"arch","x86_64"},
                {"language","en_US"},
                {"microphone","SingleArray"},
                {"vehicleIdentifier","123456789a"},
                {"engineType","GAS"},
                {"rseEmbeddedFireTvs","1"}
            }}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createVehicleInfoConfig(
        {{VehiclePropertyType::MAKE, "Honda"},
         {VehiclePropertyType::MODEL, "Odyssey"},
         {VehiclePropertyType::TRIM, "EX-L"},
         {VehiclePropertyType::YEAR, "2019"},
         {VehiclePropertyType::GEOGRAPHY, "US"},
         {VehiclePropertyType::VERSION, "1.0"},
         {VehiclePropertyType::OPERATING_SYSTEM, "darwin"},
         {VehiclePropertyType::HARDWARE_ARCH, "x86_64"},
         {VehiclePropertyType::LANGUAGE, "en_US"},
         {VehiclePropertyType::MICROPHONE, "SingleArray"},
         {VehiclePropertyType::VEHICLE_IDENTIFIER, "123456789a"},
         {VehiclePropertyType::ENGINE_TYPE, "GAS"},
         {VehiclePropertyType::RSE_EMBEDDED_FIRETVS, "1"}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

/**
 * Test createVehicleInfoConfig() expecting a valid VehicleConfigurationImplTest to be returned.
 */
TEST_F(VehicleConfigurationImplTest, createOperatingCountryConfig) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"operatingCountry","US"}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createOperatingCountryConfig("US");

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

/**
 * Test create() expecting a valid VehicleConfigurationImplTest to be returned.
 */
TEST_F(VehicleConfigurationImplTest, createVehicleInfoConfigWithEmptyValueString) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"info",{
                {"make","Honda"},
                {"model","Odyssey"},
                {"trim","EX-L"},
                {"year","2019"},
                {"geography","US"},
                {"version","1.0"},
                {"os",""},
                {"arch","x86_64"},
                {"language","en_US"},
                {"microphone","SingleArray"},
                {"vehicleIdentifier","123456789a"},
                {"engineType","GAS"},
                {"rseEmbeddedFireTvs","1"}
            }}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createVehicleInfoConfig(
        {{VehiclePropertyType::MAKE, "Honda"},
         {VehiclePropertyType::MODEL, "Odyssey"},
         {VehiclePropertyType::TRIM, "EX-L"},
         {VehiclePropertyType::YEAR, "2019"},
         {VehiclePropertyType::GEOGRAPHY, "US"},
         {VehiclePropertyType::VERSION, "1.0"},
         {VehiclePropertyType::OPERATING_SYSTEM, ""},
         {VehiclePropertyType::HARDWARE_ARCH, "x86_64"},
         {VehiclePropertyType::LANGUAGE, "en_US"},
         {VehiclePropertyType::MICROPHONE, "SingleArray"},
         {VehiclePropertyType::VEHICLE_IDENTIFIER, "123456789a"},
         {VehiclePropertyType::ENGINE_TYPE, "GAS"},
         {VehiclePropertyType::RSE_EMBEDDED_FIRETVS, "1"}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}

/**
 * Test create() expecting a valid VehicleConfigurationImplTest to be returned.
 */
TEST_F(VehicleConfigurationImplTest, createVehicleInfoConfigWithPossibleCharsinValueString) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration;
    std::ostringstream configStr;
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;

    // clang-format off
    json::Value config = {
        {"aace.vehicle",{
            {"info",{
                {"make","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"model","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"trim","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"year","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"geography","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"version","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"os","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"arch","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"language","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"microphone","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"vehicleIdentifier","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"engineType","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
                {"rseEmbeddedFireTvs","ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"}
            }}
        }}
    };
    // clang-format on

    testConfiguration = aace::vehicle::config::VehicleConfiguration::createVehicleInfoConfig(
        {{VehiclePropertyType::MAKE, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::MODEL, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::TRIM, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::YEAR, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::GEOGRAPHY, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::VERSION, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::OPERATING_SYSTEM, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::HARDWARE_ARCH, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::LANGUAGE, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::MICROPHONE, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::VEHICLE_IDENTIFIER, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::ENGINE_TYPE, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"},
         {VehiclePropertyType::RSE_EMBEDDED_FIRETVS, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"}});

    // Convert to JSON for object comparison
    configStr << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(true, config == json::toJson(configStr.str())) << "Error in the Configuration String";
}
