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
#include "AACE/Engine/Core/EngineMacros.h"

#include <AACE/Test/Alexa/AlexaTestHelper.h>
#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/test/MockAVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include <AVSCommon/SDKInterfaces/test/MockSpeakerManager.h>
#include <CertifiedSender/CertifiedSender.h>
#include <AACE/Test/AVS/MockAlertsAudioFactoryInterface.h>
#include <AACE/Test/AVS/MockCapabilitiesDelegateInterface.h>
#include <AACE/Test/AVS/MockCustomerDataManager.h>
#include <AACE/Test/AVS/MockMessageObserver.h>
#include <AACE/Test/AVS/MockMessageRouter.h>
#include <AACE/Test/AVS/MockMessageStorage.h>
#include <MockDeviceSettingStorage.h>


#include <RegistrationManager/CustomerDataManager.h>


#include <AACE/Test/AVS/MockCapabilitiesDelegateInterface.h>

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
    MOCK_METHOD1( setDoNotDisturb, void( const bool doNotDisturb ) );
    MOCK_METHOD1( doNotDisturbChanged, void( const bool doNotDisturb ));
};


class DoNotDisturbEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        }

        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();
        
        // initialize the avs device SDK
        ASSERT_TRUE( alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize( { AlexaTestHelper::getAVSConfig() } ) ) << "Initialize AVS Device SDK Failed!";
        
        auto m_mockMessageRouter = std::make_shared<testing::StrictMock<aace::test::avs::MockMessageRouter>>();
        EXPECT_CALL(*m_mockMessageRouter, setObserver(testing::_)).Times(1);
        EXPECT_CALL(*m_mockMessageRouter, enable()).Times(1);
        m_mockConnectionManager = AVSConnectionManager::create(
            m_mockMessageRouter,
            true,
            std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>(),
            std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface>>());

        m_mockMessageSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>>();

        auto m_mockDoNotDisturbPlatformInterface = std::make_shared<aace::test::unit::MockDoNotDisturbPlatformInterface>();
        auto m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        auto m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<aace::test::avs::MockCapabilitiesDelegateInterface>>();
        auto m_customerDataManager = std::make_shared<testing::StrictMock<aace::test::avs::MockCustomerDataManager>>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();

        m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
            m_mockDoNotDisturbPlatformInterface,
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_alexaMockFactory->getAVSConnectionManagerMock(),
            m_alexaMockFactory->getCustomerDataManagerMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            *m_alexaMockFactory->getDeviceSettingsDelegateMock());

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
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
       
        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> m_doNotDisturbEngineImpl;

    std::shared_ptr<aace::test::unit::MockDoNotDisturbPlatformInterface> m_mockDoNotDisturbPlatformInterface;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_mockConnectionManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>> m_mockMessageSender;
    std::shared_ptr<DeviceSettingsManager> m_settingsManager;
    std::shared_ptr<alexaClientSDK::capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent> m_dndCA;
protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;
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
TEST_F(DoNotDisturbEngineImplTest, createWithEndpointBuilderAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
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
TEST_F(DoNotDisturbEngineImplTest, createWithConnectionManagerAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
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
TEST_F(DoNotDisturbEngineImplTest, createWithCustomerDataManagerAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
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
TEST_F(DoNotDisturbEngineImplTest, createWithExceptionEncounteredSenderInterfaceAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
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
TEST_F(DoNotDisturbEngineImplTest, createWithMessageSenderAsNull)
{
    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
                                                                                   m_mockDoNotDisturbPlatformInterface,
                                                                                   m_alexaMockFactory->getEndpointBuilderMock(),
                                                                                   m_alexaMockFactory->getAVSConnectionManagerMock(),
                                                                                   m_alexaMockFactory->getCustomerDataManagerMock(),
                                                                                   m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
                                                                                   nullptr,
                                                                                   *m_alexaMockFactory->getDeviceSettingsDelegateMock());
    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
//TEST_F(DoNotDisturbEngineImplTest, createWithDeviceSettingsDelegateAsNull)
//{
//    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
//    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
//                                                                                   m_mockDoNotDisturbPlatformInterface,
//                                                                                   m_alexaMockFactory->getEndpointBuilderMock(),
//                                                                                   m_alexaMockFactory->getAVSConnectionManagerMock(),
//                                                                                   m_alexaMockFactory->getCustomerDataManagerMock(),
//                                                                                   m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
//                                                                                   m_alexaMockFactory->getMessageSenderInterfaceMock(),
//                                                                                   nullptr);
//    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
//}

///**
// * Test create() with nullptr parameters
// */
//TEST_F(DoNotDisturbEngineImplTest, createWithCapabilitiesDelegateAsNull)
//{
//    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
//    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
//                                                                                   m_mockDoNotDisturbPlatformInterface,
//                                                                                   m_alexaMockFactory->getEndpointBuilderMock(),
//                                                                                   m_alexaMockFactory->getAVSConnectionManagerMock(),
//                                                                                   m_alexaMockFactory->getCustomerDataManagerMock(),
//                                                                                   m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
//                                                                                   m_alexaMockFactory->getMessageSenderInterfaceMock(),
//                                                                                   *m_alexaMockFactory->getDeviceSettingsDelegateMock());
//    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
//}
//
///**
// * Test create() with nullptr parameters
// */
//TEST_F(DoNotDisturbEngineImplTest, createWithCapabilitiesDelegateAsNull)
//{
//    std::shared_ptr<aace::engine::alexa::DoNotDisturbEngineImpl> DoNotDisturbEngineImplTemp;
//    m_doNotDisturbEngineImpl = aace::engine::alexa::DoNotDisturbEngineImpl::create(
//                                                                                   m_mockDoNotDisturbPlatformInterface,
//                                                                                   m_alexaMockFactory->getEndpointBuilderMock(),
//                                                                                   m_alexaMockFactory->getAVSConnectionManagerMock(),
//                                                                                   m_alexaMockFactory->getCustomerDataManagerMock(),
//                                                                                   m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
//                                                                                   m_alexaMockFactory->getMessageSenderInterfaceMock(),
//                                                                                   *m_alexaMockFactory->getDeviceSettingsDelegateMock());
//    EXPECT_EQ(nullptr, DoNotDisturbEngineImplTemp) << "DoNotDisturbEngineImpl should be null";
//}

}  // namespace unit
}  // namespace test
}  // namespace aace

