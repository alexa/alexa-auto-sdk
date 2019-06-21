/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>

#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>

#include "AACE/PhoneCallController/PhoneCallController.h"
#include "AACE/Engine/PhoneCallController/PhoneCallControllerEngineImpl.h"

namespace aace {
namespace test {
namespace unit {

class MockCapabilitiesDelegate : public alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface {
public:
    MOCK_METHOD1( registerCapability, bool(const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface>& capability) );
    MOCK_METHOD0( publishCapabilities, CapabilitiesPublishReturnCode() );
    MOCK_METHOD0( invalidateCapabilities, void() );
    MOCK_METHOD0( publishCapabilitiesAsyncWithRetries, void());
    MOCK_METHOD1( addCapabilitiesObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface> observer) );
    MOCK_METHOD1( removeCapabilitiesObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface> observer) );
};

class MockPhoneCallControllerPlatformInterface : public aace::phoneCallController::PhoneCallController {
public:
    MOCK_METHOD1( dial, bool(const std::string& payload) );
    MOCK_METHOD1( redial, bool(const std::string& payload) );
    MOCK_METHOD1( stop, void(const std::string& payload) );
    MOCK_METHOD1( answer, void(const std::string& payload) );
    MOCK_METHOD1( playRingtone, void(const std::string& payload) );
    MOCK_METHOD1( sendDTMF, void(const std::string& payload) );
    MOCK_METHOD1( connectionStateChanged, void(aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state) );
    MOCK_METHOD3( callStateChanged, void(aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state, const std::string& callId, const std::string& callerId) );
    MOCK_METHOD3( callFailed, void(const std::string& callId, aace::phoneCallController::PhoneCallControllerEngineInterface::CallError code, const std::string& message) );
    MOCK_METHOD2( callerIdReceived, void(const std::string& callId, const std::string& callerId) );
    MOCK_METHOD1( sendDTMFSucceeded, void(const std::string& callId) );
    MOCK_METHOD3( sendDTMFFailed, void(const std::string& callId, aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError code, const std::string& message) );
    MOCK_METHOD1( deviceConfigurationUpdated, void(std::unordered_map<aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty, bool> configurationMap) );
    MOCK_METHOD0( createCallId, void() );
};

class PhoneCallControllerEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockPlatformInterface = std::make_shared<testing::StrictMock<MockPhoneCallControllerPlatformInterface>>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<MockCapabilitiesDelegate>>();
        m_mockContextManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockMessageSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>>();
        m_mockFocusManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>>();

        EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS));
        m_engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
            m_mockPlatformInterface,
            m_mockCapabilitiesDelegate,
            m_mockContextManager,
            m_mockDirectiveSequencer,
            m_mockExceptionSender,
            m_mockMessageSender,
            m_mockFocusManager
        );
    }
    void TearDown() override {
        m_engineImpl->shutdown();
    }

    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> m_engineImpl;
    std::shared_ptr<aace::phoneCallController::PhoneCallController> m_mockPlatformInterface;
    std::shared_ptr<testing::StrictMock<MockCapabilitiesDelegate>> m_mockCapabilitiesDelegate;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>> m_mockContextManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>> m_mockMessageSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>> m_mockFocusManager;
};

TEST_F( PhoneCallControllerEngineImplTest, create ) {
    EXPECT_NE(nullptr, m_engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullPlatform ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl; 
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        nullptr, m_mockCapabilitiesDelegate, m_mockContextManager, m_mockDirectiveSequencer, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullCapabilitiesDelegate ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, nullptr, m_mockContextManager, m_mockDirectiveSequencer, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullContextManager ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_mockCapabilitiesDelegate, nullptr, m_mockDirectiveSequencer, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullDirectiveSequencer ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_mockCapabilitiesDelegate, m_mockContextManager, nullptr, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullExceptionSender ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_mockCapabilitiesDelegate, m_mockContextManager, m_mockDirectiveSequencer, nullptr, m_mockMessageSender, m_mockFocusManager );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullMessageSender ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_mockCapabilitiesDelegate, m_mockContextManager, m_mockDirectiveSequencer, m_mockExceptionSender, nullptr, m_mockFocusManager );
    EXPECT_EQ(nullptr, engineImpl);
}

} // aace::test::unit
} // aace::test
} // aace
