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

#include "AACE/Engine/Alexa/DoNotDisturbEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"


#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/test/MockAVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include "include/MockSpeakerManager.h"
#include <CertifiedSender/CertifiedSender.h>
#include "include/MockAlertsAudioFactoryInterface.h"
#include "include/MockCapabilitiesDelegateInterface.h"
#include "include/MockCustomerDataManager.h"
#include "include/MockMediaPlayer.h"
#include "include/MockMessageObserver.h"
#include "include/MockMessageRouter.h"
#include "include/MockMessageStorage.h"
#include "include/MockSpeaker.h"
#include "include/MockDeviceSettingStorage.h"


#include <RegistrationManager/CustomerDataManager.h>


#include "include/MockCapabilitiesDelegateInterface.h"

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
using namespace alexaClientSDK::settings::storage::test;
using namespace alexaClientSDK::settings;
using ::testing::Return;



class MockDoNotDisturbPlatformInteraface : public aace::alexa::DoNotDisturb {
public:
    MockDoNotDisturbPlatformInteraface();
    MOCK_METHOD1( setDoNotDisturb, void( const bool doNotDisturb ) );
    MOCK_METHOD1( doNotDisturbChanged, void( const bool doNotDisturb ))
};


class DoNotDisturbEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        }

       
        m_mockMessageRouter = std::make_shared<testing::StrictMock<aace::test::unit::MockMessageRouter>>();
        EXPECT_CALL(*m_mockMessageRouter, setObserver(testing::_)).Times(1);
        EXPECT_CALL(*m_mockMessageRouter, enable()).Times(1);
        m_mockConnectionManager = AVSConnectionManager::create(
            m_mockMessageRouter,
            true,
            std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>(),
            std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface>>());

        m_mockMessageSender = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>();

        m_mockDoNotDisturbPlatformInterface = std::make_shared<aace::test::unit::MockDoNotDisturbPlatformInteraface>();
        m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::DirectiveSequencer>>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>>();
        m_customerDataManager = std::make_shared<testing::StrictMock<MockCustomerDataManager>>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<aace::test::unit::MockExceptionSender>>();
        m_settingsManager = std::make_shared<DeviceSettingsManager>();
        m_settingsStorage = std::make_shared<MockDeviceSettingStorage>();

        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));

        m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
            m_mockDoNotDisturbPlatformInterface,
            m_mockDirectiveSequencer,
            m_mockConnectionManager,
            m_mockCapabilitiesDelegate,
            m_customerDataManager,
            m_mockExceptionSender,
            m_mockConnectionManager,
            m_settingsManager,
            m_settingsStorage );

        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        }
    }

    void TearDown() override {
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        }

        if( m_doNotDisturbEngineImpl  != nullptr ) {
            m_doNotDisturbEngineImpl->shutdown();
        }
       
        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> m_doNotDisturbEngineImpl;

    std::shared_ptr<aace::test::unit::MockDoNotDisturbPlatformInterface> m_mockDoNotDIsturbPlatformInterface;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_mockConnectionManager;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>> m_mockCapabilitiesDelegate;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCustomerDataManager>> m_customerDataManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>> m_mockMessageSender;
    std::shared_ptr<DeviceSettingsManager> m_settingsManager;
    std::shared_ptr<MockDeviceSettingStorage> m_settingsStorage;
    std::shared_ptr<DoNotDisturbCapabilityAgent> m_dndCA;
};

/**
 * Test create() with valid parameters
 */
TEST_F( DoNotDisturbEngineImplTest, create ) {
    EXPECT_NE(nullptr, m_doNotDisturbEngineImpl) << "DoNotDisturbEngineImpl pointer is null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithDoNotDisturbAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        nullptr,
        m_mockDirectiveSequencer,
        m_mockConnectionManager,
        m_mockCapabilitiesDelegate,
        m_customerDataManager,
        m_mockExceptionSender,
        m_mockConnectionManager,
        m_settingsManager,
        m_settingsStorage );
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithMessageSenderAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        nullptr,
        m_mockConnectionManager,
        m_mockCapabilitiesDelegate,
        m_customerDataManager,
        m_mockExceptionSender,
        m_mockConnectionManager,
        m_settingsManager,
        m_settingsStorage );
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithContextManagerAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_mockDirectiveSequencer,
        nullptr,
        m_mockCapabilitiesDelegate,
        m_customerDataManager,
        m_mockExceptionSender,
        m_mockConnectionManager,
        m_settingsManager,
        m_settingsStorage );
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithCapabilitiesDelegateAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_mockDirectiveSequencer,
        m_mockConnectionManager,
        nullptr,
        m_customerDataManager,
        m_mockExceptionSender,
        m_mockConnectionManager,
        m_settingsManager,
        m_settingsStorage);
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithCapabilitiesDelegateAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_mockDirectiveSequencer,
        m_mockConnectionManager,
        m_mockCapabilitiesDelegate,
        nullptr,
        m_mockExceptionSender,
        m_mockConnectionManager,
        m_settingsManager,
        m_settingsStorage);
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithCapabilitiesDelegateAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_mockDirectiveSequencer,
        m_mockConnectionManager,
        m_mockCapabilitiesDelegate,
        m_customerDataManager,
        nullptr,
        m_mockConnectionManager,
        m_settingsManager,
        m_settingsStorage);
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithCapabilitiesDelegateAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_mockDirectiveSequencer,
        m_mockConnectionManager,
        m_mockCapabilitiesDelegate,
        m_customerDataManager,
        m_mockExceptionSender,
        nullptr,
        m_settingsManager,
        m_settingsStorage);
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithCapabilitiesDelegateAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_mockDirectiveSequencer,
        m_mockConnectionManager,
        m_mockCapabilitiesDelegate,
        m_customerDataManager,
        m_mockExceptionSender,
        m_mockConnectionManager,
        nullptr,
        m_settingsStorage);
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(DoNotDisturbEngineImplTest, createWithCapabilitiesDelegateAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
        m_mockDoNotDisturbPlatformInterface,
        m_mockDirectiveSequencer,
        m_mockConnectionManager,
        m_mockCapabilitiesDelegate,
        m_customerDataManager,
        m_mockExceptionSender,
        m_mockConnectionManager,
        m_settingsManager,
        nullptr);
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

}  // namespace unit
}  // namespace test
}  // namespace aace

