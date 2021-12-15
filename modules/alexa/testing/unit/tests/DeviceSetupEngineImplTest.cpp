/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Test/Unit/Alexa/AlexaMockComponentFactory.h>
#include <AACE/Test/Unit/Alexa/AlexaTestHelper.h>
#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "AACE/Alexa/DeviceSetup.h"
#include "AACE/Engine/Alexa/DeviceSetupEngineImpl.h"
#include "AACE/Test/Unit/Alexa/MockDeviceSetup.h"

using namespace aace::test::unit::alexa;

namespace aace {
namespace test {
namespace unit {

using ::testing::_;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::Test;
using ::testing::TestWithParam;
using ::testing::Values;

class DeviceSetupEngineImplTest : public TestWithParam<std::pair<std::string, bool>> {
public:
    void SetUp() override {
        m_alexaMockComponentFactory = aace::test::unit::alexa::AlexaTestHelper::createAlexaMockComponentFactory();

        // Initialize the AVS Device SDK.
        ASSERT_TRUE(alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize(
            {aace::test::unit::alexa::AlexaTestHelper::getAVSConfig()}))
            << "Initialize AVS Device SDK Failed!";

        // Mock the DeviceSetupHandler and configuration data.
        m_mockDeviceSetupHandler = std::make_shared<StrictMock<alexa::MockDeviceSetup>>();

        // Initialization succeeded.
        m_initialized = true;
    }

    void TearDown() override {
        if (m_initialized) {
            m_alexaMockComponentFactory->shutdown();

            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();

            m_initialized = false;
        }
    }

protected:
    void configure() {
        if (m_configured == false) {
            m_configured = true;
        }
    }

    std::shared_ptr<aace::engine::alexa::DeviceSetupEngineImpl> createDeviceSetupEngineImpl(
        std::shared_ptr<StrictMock<alexa::MockDeviceSetup>> mockDeviceSetupHandler) {
        if (m_configured == false) {
            configure();
        }

        auto deviceSetupEngineImpl = aace::engine::alexa::DeviceSetupEngineImpl::create(
            mockDeviceSetupHandler, m_alexaMockComponentFactory->getMessageSenderInterfaceMock());

        return deviceSetupEngineImpl;
    }

protected:
    std::shared_ptr<aace::test::unit::alexa::AlexaMockComponentFactory> m_alexaMockComponentFactory;
    std::shared_ptr<StrictMock<alexa::MockDeviceSetup>> m_mockDeviceSetupHandler;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

TEST_F(DeviceSetupEngineImplTest, create) {
    auto deviceSetupEngineImpl = createDeviceSetupEngineImpl(m_mockDeviceSetupHandler);
    ASSERT_NE(deviceSetupEngineImpl, nullptr) << "DeviceSetupEngineImpl pointer expected to be not null";

    deviceSetupEngineImpl->shutdown();
}

TEST_F(DeviceSetupEngineImplTest, createWithDeviceSetupHandlerAsNull) {
    EXPECT_CALL(*m_alexaMockComponentFactory->getDirectiveSequencerInterfaceMock(), doShutdown());
    std::shared_ptr<aace::engine::alexa::DeviceSetupEngineImpl> DeviceSetupEngineImplTemp;
    DeviceSetupEngineImplTemp = aace::engine::alexa::DeviceSetupEngineImpl::create(
        nullptr, m_alexaMockComponentFactory->getMessageSenderInterfaceMock());

    EXPECT_EQ(nullptr, DeviceSetupEngineImplTemp) << "DeviceSetupEngineImpl should be null";
}

TEST_F(DeviceSetupEngineImplTest, createWithMessageSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::DeviceSetupEngineImpl> DeviceSetupEngineImplTemp;
    DeviceSetupEngineImplTemp = aace::engine::alexa::DeviceSetupEngineImpl::create(m_mockDeviceSetupHandler, nullptr);

    EXPECT_EQ(nullptr, DeviceSetupEngineImplTemp) << "DeviceSetupEngineImpl should be null";
}

TEST_F(DeviceSetupEngineImplTest, setupCompletedAPICall) {
    auto deviceSetupEngineImpl = createDeviceSetupEngineImpl(m_mockDeviceSetupHandler);
    deviceSetupEngineImpl->onSetupCompleted();

    ASSERT_NE(deviceSetupEngineImpl, nullptr) << "Application should not be crashed";
}

}  // namespace unit
}  // namespace test
}  // namespace aace
