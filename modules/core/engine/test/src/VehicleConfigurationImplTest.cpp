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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

#include "AACE/Vehicle/VehicleConfiguration.h"

namespace aace {
namespace engine {
namespace test {
namespace vehicle {

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
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string& temp =
        "{\n"
        "    \"aace.vehicle\": {\n"
        "        \"info\": {\n"
        "            \"make\": \"Honda\",\n"
        "            \"model\": \"Odyssey\",\n"
        "            \"trim\": \"EX-L\",\n"
        "            \"year\": \"2019\",\n"
        "            \"geography\": \"US\",\n"
        "            \"version\": \"1.0\",\n"
        "            \"os\": \"darwin\",\n"
        "            \"arch\": \"x86_64\",\n"
        "            \"language\": \"en_US\",\n"
        "            \"microphone\": \"SingleArray\",\n"
        "            \"vehicleIdentifier\": \"123456789a\"\n"
        "        }\n"
        "    }\n"
        "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
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
         {VehiclePropertyType::VEHICLE_IDENTIFIER, "123456789a"}});

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createVehicleInfoConfig() expecting a valid VehicleConfigurationImplTest to be returned.
 */
TEST_F(VehicleConfigurationImplTest, createOperatingCountryConfig) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string& temp =
        "{\n"
        "    \"aace.vehicle\": {\n"
        "        \"operatingCountry\": \"US\"\n"
        "    }\n"
        "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = aace::vehicle::config::VehicleConfiguration::createOperatingCountryConfig("US");

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test create() expecting a valid VehicleConfigurationImplTest to be returned.
 */
TEST_F(VehicleConfigurationImplTest, createVehicleInfoConfigWithEmptyValueString) {
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string& temp =
        "{\n"
        "    \"aace.vehicle\": {\n"
        "        \"info\": {\n"
        "            \"make\": \"Honda\",\n"
        "            \"model\": \"Odyssey\",\n"
        "            \"trim\": \"EX-L\",\n"
        "            \"year\": \"2019\",\n"
        "            \"geography\": \"US\",\n"
        "            \"version\": \"1.0\",\n"
        "            \"os\": \"\",\n"
        "            \"arch\": \"x86_64\",\n"
        "            \"language\": \"en_US\",\n"
        "            \"microphone\": \"SingleArray\",\n"
        "            \"vehicleIdentifier\": \"123456789a\"\n"
        "        }\n"
        "    }\n"
        "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
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
         {VehiclePropertyType::VEHICLE_IDENTIFIER, "123456789a"}});

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test create() expecting a valid VehicleConfigurationImplTest to be returned.
 */
TEST_F(VehicleConfigurationImplTest, createVehicleInfoConfigWithPossibleCharsinValueString) {
    using VehiclePropertyType = aace::vehicle::config::VehicleConfiguration::VehiclePropertyType;
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string& temp =
        "{\n"
        "    \"aace.vehicle\": {\n"
        "        \"info\": {\n"
        "            \"make\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"model\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"trim\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"year\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"geography\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"version\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"os\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"arch\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"language\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"microphone\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "            \"vehicleIdentifier\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
        "        }\n"
        "    }\n"
        "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
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
         {VehiclePropertyType::VEHICLE_IDENTIFIER, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?"}});

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

}  // namespace vehicle
}  // namespace test
}  // namespace engine
}  // namespace aace
