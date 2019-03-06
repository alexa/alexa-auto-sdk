/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

#include "AACE/Alexa/AlexaConfiguration.h"
#include "AACE/Engine/Core/EngineMacros.h"

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

using namespace  aace::alexa::config;

class AlexaConfigurationImplTest : public ::testing::Test {
public:
    void SetUp() override {
        //Do Nothing. Just kept for using TEST_F framework.
        if (DEBUG){
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }
};

/**
 * Test createDeviceConfiguration for best case
 */
TEST_F( AlexaConfigurationImplTest, createDeviceInfoConfigBestCase ) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string& deviceSerialNumber = "123456789";
    const std::string& clientId = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string& productId ="ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string& temp = "{\n"
        "    \"deviceInfo\": {\n"
        "        \"deviceSerialNumber\": \"123456789\",\n"
        "        \"clientId\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
        "        \"productId\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
        "    }\n"
        "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createDeviceInfoConfig( 
        deviceSerialNumber, clientId, productId );

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString,actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString<<testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0,expectedString.str().compare(actualString.str()))<<"Error in the Configuration String";
}

/**
 * Test createAlertsConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createAlertsConfigBestCase) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &databaseFilePath = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &temp = "{\n"
                              "    \"alertsCapabilityAgent\": {\n"
                              "        \"databaseFilePath\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
                              "    }\n"
                              "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createAlertsConfig(databaseFilePath);

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createNotificationsConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createNotificationsConfigBestCase) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &databaseFilePath = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &temp = "{\n"
                              "    \"notifications\": {\n"
                              "        \"databaseFilePath\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
                              "    }\n"
                              "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createNotificationsConfig(databaseFilePath);

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createCertifiedSenderConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createCertifiedSenderConfigBestCase) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &databaseFilePath = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &temp = "{\n"
                              "    \"certifiedSender\": {\n"
                              "        \"databaseFilePath\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
                              "    }\n"
                              "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createCertifiedSenderConfig(databaseFilePath);

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createCurlConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createCurlConfigBestCase) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &databaseFilePath = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &temp = "{\n"
                              "    \"libcurlUtils\": {\n"
                              "        \"CURLOPT_CAPATH\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
                              "    }\n"
                              "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createCurlConfig(databaseFilePath);

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createCurlConfig() with non empty network interface name
 */
TEST_F(AlexaConfigurationImplTest, createCurlConfigWithNetworkInterface)
{
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &databaseFilePath = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &optInterface = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &temp = "{\n"
                              "    \"libcurlUtils\": {\n"
                              "        \"CURLOPT_CAPATH\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
                              "        \"CURLOPT_INTERFACE\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
                              "    }\n"
                              "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createCurlConfig(databaseFilePath, optInterface);

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createCurlConfig() with empty string network interface name
 */
TEST_F(AlexaConfigurationImplTest, createCurlConfigWithEmptyStringNetworkInterface)
{
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &databaseFilePath = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &optInterface = ""; //Empty String
    const std::string &temp = "{\n"
                              "    \"libcurlUtils\": {\n"
                              "        \"CURLOPT_CAPATH\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
                              "    }\n"
                              "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createCurlConfig(databaseFilePath, optInterface);

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createSettingsConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createSettingsConfigBestCase) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &databaseFilePath = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &locale = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &temp =   "{\n"
                                "    \"settings\": {\n"
                                "        \"databaseFilePath\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\",\n"
                                "        \"defaultAVSClientSettings\": {\n"
                                "            \"locale\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
                                "        }\n"
                                "    }\n"
                                "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createSettingsConfig(databaseFilePath, locale);

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    std::cout << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createSystemConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createSystemConfigBestCase) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const uint32_t firmwareVersion = 1234567890;
    const std::string &temp = "{\n"
                              "    \"aace.alexa\": {\n"
                              "        \"system\": {\n"
                              "            \"firmwareVersion\": 1234567890\n"
                              "        }\n"
                              "    }\n"
                              "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createSystemConfig(firmwareVersion);

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createMiscStorageConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createMiscStorageConfigBestCase) {
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &databaseFilePath = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    const std::string &temp = "{\n"
                              "    \"miscDatabase\": {\n"
                              "        \"databaseFilePath\": \"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?\"\n"
                              "    }\n"
                              "}";

    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createMiscStorageConfig(databaseFilePath);

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createTemplateRuntimeConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createTemplateRuntimeConfigBestCase)
{
    using TemplateRuntimeTimeoutType = alexa::config::AlexaConfiguration::TemplateRuntimeTimeoutType;
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &temp = "{\n"
                              "    \"templateRuntimeCapabilityAgent\": {\n"
                              "        \"displayCardTTSFinishedTimeout\": 1234509876,\n"
                              "        \"displayCardAudioPlaybackFinishedTimeout\": 1234567890,\n"
                              "        \"displayCardAudioPlaybackStoppedPausedTimeout\": 987654321\n"
                              "    }\n"
                              "}";
    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createTemplateRuntimeTimeoutConfig({
        { TemplateRuntimeTimeoutType::DISPLAY_CARD_TTS_FINISHED_TIMEOUT, std::chrono::milliseconds( 1234509876 ) },
        { TemplateRuntimeTimeoutType::DISPLAY_CARD_AUDIO_PLAYBACK_FINISHED_TIMEOUT, std::chrono::milliseconds( 1234567890 ) },
        { TemplateRuntimeTimeoutType::DISPLAY_CARD_AUDIO_PLAYBACK_STOPPED_PAUSED_TIMEOUT, std::chrono::milliseconds( 987654321 ) }
    });

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createTemplateRuntimeConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createTemplateRuntimeConfigWithOneKeyBestCase)
{
    using TemplateRuntimeTimeoutType = alexa::config::AlexaConfiguration::TemplateRuntimeTimeoutType;
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &temp = "{\n"
                              "    \"templateRuntimeCapabilityAgent\": {\n"
                              "        \"displayCardTTSFinishedTimeout\": 1234509876\n"
                              "    }\n"
                              "}";
    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createTemplateRuntimeTimeoutConfig({
        { TemplateRuntimeTimeoutType::DISPLAY_CARD_TTS_FINISHED_TIMEOUT, std::chrono::milliseconds( 1234509876 ) }
    });

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

/**
 * Test createTemplateRuntimeConfig() for best case
 */
TEST_F(AlexaConfigurationImplTest, createTemplateRuntimeConfigWithNoKeys)
{
    std::shared_ptr<aace::core::config::EngineConfiguration> testConfiguration, expectedConfiguration;
    const std::string &temp = "{\n"
                              "    \"templateRuntimeCapabilityAgent\": {}\n"
                              "}";
    expectedConfiguration = aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(temp));
    testConfiguration = alexa::config::AlexaConfiguration::createTemplateRuntimeTimeoutConfig({/*Empty List*/});

    //Convert to ostringstream for comparing the istream
    std::ostringstream expectedString, actualString;
    expectedString << expectedConfiguration->getStream()->rdbuf();
    actualString << testConfiguration->getStream()->rdbuf();
    EXPECT_EQ(0, expectedString.str().compare(actualString.str())) << "Error in the Configuration String";
}

}  // namespace unit
}  // namespace test
}  // namespace aace

