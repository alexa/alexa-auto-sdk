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
#include <gtest/gtest.h>

#include <aasb/Consts.h>
#include <aasb/AASBControllerFactory.h>
#include <aasb/interfaces/IConfigurationProvider.h>

#include "MockEngine.h"

namespace aasb {
namespace bridge {

/// Base path of service where all configs, app data etc. ought to be stored.
const static std::string SVC_BASE_PATH = "/base/path/to/mock/svc/";

/// Path to the alexa configuration.
const static std::string CONFIG_DIR = SVC_BASE_PATH + "config";

/// Path to the app's certificates.
const static std::string CERTS_DIR = SVC_BASE_PATH + "certs";

/// Path to the app's data.
const static std::string APP_DATA_DIR = SVC_BASE_PATH + "app-data";

/// Product device serial number to identify the Alexa device instance.
const static std::string PRODUCT_DSN = "dummy-product-dsn";

/// Product id to identify the Alexa device category.
const static std::string PRODUCT_ID = "dummy-product-id";

/// Client id to identify the current user logged into the device.
const static std::string CLIENT_ID = "dummy-client-id";

/// Country information.
const static std::string COUNTRY = "US";

/// Coordinates of Vegas
const static float COOR_LAT = 36.115;
const static float COOR_LONG = -115.173;

class ConfigurationProvider : public IConfigurationProvider {
public:
    ConfigurationProvider() {}
    AudioIOConfiguration getAudioIOConfig() {
        return mAudioIOConfig; 
    }
    std::string getCertificatesDirectoryPath() { return CERTS_DIR; }
    std::string getAppsDataDirectory() { return APP_DATA_DIR; }
    std::string getProductDSN() { return PRODUCT_DSN; }
    std::string getClientId() { return CLIENT_ID; }
    std::string getProductId() { return PRODUCT_ID; }
    bool shouldEnablePhoneCallControl() { return true; }
    bool shouldEnableNavigation() { return true; }
    bool shouldEnableWakeword() { return true; }
    bool shouldEnableCBL() { return true; }
    bool shouldEnableGloriaCard() { return true; }
    bool shouldEnableGloriaList() { return true; }
    bool shouldEnableClimateControl() { return true; }
    std::pair<float, float> getCurrentLocation() { return std::make_pair(COOR_LAT,COOR_LONG); }
    std::string getCountry() { return COUNTRY; }
private:
    AudioIOConfiguration mAudioIOConfig;
};


/**
 * Test for @c AASBController
 */
class AASBControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mMockEngine = std::make_shared<aace::core::MockEngine>();
        mAASBController = AASBControllerFactory::createAASBController();
        mAASBController->setMockEngine(mMockEngine);
    }

    /**
     * Initialize the @c mAASBController instance.
     *
     * @return true on success, false otherwise.
     */
    bool initialize();

    std::shared_ptr<aace::core::MockEngine> mMockEngine;
    std::shared_ptr<IAASBController> mAASBController;
};

bool AASBControllerTest::initialize() {
    ON_CALL(*mMockEngine, configure(
        testing::Matcher<std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>>>(testing::_)))
        .WillByDefault(testing::Return(true));
    EXPECT_CALL(*mMockEngine, configure(
        testing::Matcher<std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>>>(testing::_)))
        .Times(1);
    ON_CALL(*mMockEngine, start())
        .WillByDefault(testing::Return(true));
    EXPECT_CALL(*mMockEngine, start())
        .Times(1);
    ON_CALL(*mMockEngine, registerPlatformInterface(
        testing::Matcher<std::shared_ptr<aace::core::PlatformInterface>>(testing::_)))
        .WillByDefault(testing::Return(true));
    EXPECT_CALL(*mMockEngine, registerPlatformInterface(
        testing::Matcher<std::shared_ptr<aace::core::PlatformInterface>>(testing::_)))
        .Times(13);
    // AASB should register 13 platform interfaces as of now. Change this number as we
    // register more platform interfaces to engine.

    if(!mAASBController->init(std::shared_ptr<IConfigurationProvider>(new ConfigurationProvider())))
        return false;
    return mAASBController->start();
}

/**
 * Test that @c AASBController APIs returns error before it is initialized
 */
TEST_F(AASBControllerTest, errorsOnCallsBeforeInitialize) {
    std::vector<int16_t> fakeData {1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_FALSE(mAASBController->writeAudioSamples(fakeData.data(), fakeData.size()));

    EXPECT_CALL(*mMockEngine, setProperty(testing::_, testing::_)).Times(3);

    // Make sure following setters do not crash.
    mAASBController->setAVSEndPoint("dummyEndPoint");
    mAASBController->setLocale("dummyLocale");
    mAASBController->setFirmwareVersion("dummyFirmwareVersion");
}

/**
 * Test that we can initialize the @c AASBController
 */
TEST_F(AASBControllerTest, initializeWorks) {
    ASSERT_TRUE(initialize());
}

}  // namespace bridge
}  // namespace aasb