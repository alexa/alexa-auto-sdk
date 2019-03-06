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

#include "RequestProcessor.h"

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

/// Cleint id to identify the current user logged into the device.
const static std::string CLIENT_ID = "dummy-client-id";

/**
 * Test for @c RequestProcessor
 */
class RequestProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        mMockEngine = std::make_shared<aace::core::MockEngine>();
        mRequestProcessor = std::shared_ptr<RequestProcessor>(new RequestProcessor(mMockEngine));
    }

    /**
     * Initialize the @c mRequestProcessor instance.
     *
     * @return true on success, false otherwise.
     */
    bool initialize();

    std::shared_ptr<aace::core::MockEngine> mMockEngine;
    std::shared_ptr<RequestProcessor> mRequestProcessor;
};

bool RequestProcessorTest::initialize() {
    ON_CALL(*mMockEngine, configure(
        testing::Matcher<std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>>>(testing::_)))
        .WillByDefault(testing::Return(true));
    EXPECT_CALL(*mMockEngine, configure(
        testing::Matcher<std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>>>(testing::_)))
        .Times(1);
    ON_CALL(*mMockEngine, registerPlatformInterface(
        testing::Matcher<std::shared_ptr<aace::core::PlatformInterface>>(testing::_)))
        .WillByDefault(testing::Return(true));
    EXPECT_CALL(*mMockEngine, registerPlatformInterface(
        testing::Matcher<std::shared_ptr<aace::core::PlatformInterface>>(testing::_)))
        .Times(8);
    // AASB should register 8 platform interfaces as of now. Change this number as we
    // register more platform interfaces to engine.

    return mRequestProcessor->init(nullptr);
}

/**
 * Test that @c RequestProcessor APIs returns error before it is initialized
 */
TEST_F(RequestProcessorTest, errorsOnCallsBeforeInitialize) {
    ASSERT_EQ(RequestProcessor::NOT_INITALIZED_ERROR, mRequestProcessor->getConnectionStatus());
    ASSERT_EQ(RequestProcessor::NOT_INITALIZED_ERROR, mRequestProcessor->getAuthState());
    ASSERT_EQ(RequestProcessor::NOT_INITALIZED_ERROR, mRequestProcessor->getDialogState());

    std::vector<int16_t> fakeData {1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_FALSE(mRequestProcessor->writeAudioSamples(fakeData.data(), fakeData.size()));

    EXPECT_CALL(*mMockEngine, setProperty(testing::_, testing::_)).Times(3);

    // Make sure following setters do not crash.
    mRequestProcessor->setAVSEndPoint("dummyEndPoint");
    mRequestProcessor->setLocale("dummyLocale");
    mRequestProcessor->setFirmwareVersion("dummyFirmwareVersion");
}

/**
 * Test that we can initialize the @c RequestProcessor
 */
TEST_F(RequestProcessorTest, initializeWorks) {
    ASSERT_TRUE(initialize());
}

}  // namespace bridge
}  // namespace aasb