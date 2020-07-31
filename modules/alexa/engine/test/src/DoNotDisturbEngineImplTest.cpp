/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Alexa/DoNotDisturbEngineImpl.h"

#include <AACE/Test/Alexa/AlexaTestHelper.h>
#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include <AACE/Test/AVS/MockCustomerDataManager.h>
#include <MockDeviceSettingStorage.h>

#include <RegistrationManager/CustomerDataManager.h>

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::acl;
using namespace alexaClientSDK::avsCommon;
using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::sdkInterfaces::test;
using namespace alexaClientSDK::avsCommon::utils;
using namespace alexaClientSDK::registrationManager;
using namespace alexaClientSDK::settings::storage;
using namespace alexaClientSDK::settings;
using namespace aace::test::alexa;
using ::testing::Return;

class MockDoNotDisturbPlatformInterface : public aace::alexa::DoNotDisturb {
public:
    MOCK_METHOD1(setDoNotDisturb, void(const bool doNotDisturb));
    MOCK_METHOD1(doNotDisturbChanged, void(const bool doNotDisturb));
};

class DoNotDisturbEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }

        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();

        // initialize the avs device SDK
        ASSERT_TRUE(alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize(
            {AlexaTestHelper::getAVSConfig()}))
            << "Initialize AVS Device SDK Failed!";

        m_mockDoNotDisturbPlatformInterface = std::make_shared<aace::test::unit::MockDoNotDisturbPlatformInterface>();

        auto mockDeviceSettingStorage =
            std::make_shared<alexaClientSDK::settings::storage::test::MockDeviceSettingStorage>();
        EXPECT_CALL(*m_alexaMockFactory->getDeviceSettingsDelegateMock(), getDeviceSettingStorage())
            .WillOnce(Return(mockDeviceSettingStorage));
        EXPECT_CALL(*m_alexaMockFactory->getDeviceSettingsDelegateMock(), getDeviceSettingsManager())
            .WillOnce(::testing::Return(m_alexaMockFactory->getDeviceSettingsManagerMock()));

        m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
            m_mockDoNotDisturbPlatformInterface,
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_alexaMockFactory->getAVSConnectionManagerMock(),
            m_alexaMockFactory->getCustomerDataManagerMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            *m_alexaMockFactory->getDeviceSettingsDelegateMock());

        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }

        m_initialized = true;
    }

    void TearDown() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }
        if (m_initialized) {
            m_alexaMockFactory->shutdown();

            if (m_doNotDisturbEngineImpl != nullptr) {
                m_doNotDisturbEngineImpl->shutdown();
            }
            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
        }

        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> m_doNotDisturbEngineImpl;

    std::shared_ptr<aace::test::unit::MockDoNotDisturbPlatformInterface> m_mockDoNotDisturbPlatformInterface;

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;
    bool m_initialized = false;
};

/**
 * Test create() with valid parameters
 */
TEST_F(DoNotDisturbEngineImplTest, create) {
    EXPECT_NE(nullptr, m_doNotDisturbEngineImpl) << "DoNotDisturbEngineImpl pointer is null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithDoNotDisturbAsNull) {
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    DoNotDisturbEngineImplTemp = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        *m_alexaMockFactory->getDeviceSettingsDelegateMock());

    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithEndpointBuilderAsNull) {
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    DoNotDisturbEngineImplTemp = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        nullptr,
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        *m_alexaMockFactory->getDeviceSettingsDelegateMock());

    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithConnectionManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    DoNotDisturbEngineImplTemp = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_alexaMockFactory->getEndpointBuilderMock(),
        nullptr,
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        *m_alexaMockFactory->getDeviceSettingsDelegateMock());
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithCustomerDataManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    DoNotDisturbEngineImplTemp = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        nullptr,
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        *m_alexaMockFactory->getDeviceSettingsDelegateMock());
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithExceptionEncounteredSenderInterfaceAsNull) {
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    DoNotDisturbEngineImplTemp = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        nullptr,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        *m_alexaMockFactory->getDeviceSettingsDelegateMock());
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithMessageSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    DoNotDisturbEngineImplTemp = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        nullptr,
        *m_alexaMockFactory->getDeviceSettingsDelegateMock());
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

}  // namespace unit
}  // namespace test
}  // namespace aace
