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

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>

#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>

#include "AACE/Test/Alexa/AlexaTestHelper.h"
#include "AACE/PhoneCallController/PhoneCallController.h"
#include "AACE/Engine/PhoneCallController/PhoneCallControllerEngineImpl.h"

namespace aace {
namespace test {
namespace unit {

// clang-format off
static const std::string CAPABILITIES_CONFIG_JSON =
    "{"
    "    \"deviceInfo\":{"
    "        \"deviceSerialNumber\":\"MockAddressBookTest\", "
    "        \"clientId\":\"MockClientId\","
    "        \"productId\":\"MockProductID\","
    "        \"manufacturerName\":\"MockManufacturerName\","
    "        \"description\":\"MockDescription\""
    "    }"
    " }";

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
    
class MockAuthDelegateInterface : public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface {
public:
    MOCK_METHOD1( addAuthObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) );
    MOCK_METHOD1( removeAuthObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) );
    MOCK_METHOD0( getAuthToken, std::string() );
    MOCK_METHOD1( onAuthFailure, void(const std::string& token) );
};

class DummyAlexaEndpointInterface : public aace::engine::alexa::AlexaEndpointInterface {
public:
    std::string getAVSGateway() override {
        // Not called.
        return "";
    }
    std::string getLWAEndpoint() override {
        // Not called.
        return "";
    }
    std::string getACMSEndpoint() override {
        return "https://alexa-comms-mobile-service-na.amazon.com";
    }
};

class PhoneCallControllerEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        auto inString = std::shared_ptr<std::istringstream>(new std::istringstream(CAPABILITIES_CONFIG_JSON));
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize({inString});

        m_mockPlatformInterface = std::make_shared<testing::StrictMock<MockPhoneCallControllerPlatformInterface>>();
        m_mockContextManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockMessageSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>>();
        m_mockFocusManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>>();
        m_alexaMockFactory = m_alexaMockFactory = alexa::AlexaTestHelper::createAlexaMockComponentFactory();
        m_mockAuthDelegate = std::make_shared<testing::StrictMock<MockAuthDelegateInterface>>();
        m_deviceInfo = alexaClientSDK::avsCommon::utils::DeviceInfo::create(alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot());
        m_alexaEndpointInterface = std::make_shared<DummyAlexaEndpointInterface>();

        EXPECT_CALL(*m_mockAuthDelegate, addAuthObserver(testing::_)).WillOnce(testing::Return());
        EXPECT_CALL(*m_mockAuthDelegate, removeAuthObserver(testing::_)).WillOnce(testing::Return());
        EXPECT_CALL(*m_mockFocusManager, releaseChannel(testing::_, testing::_))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::InvokeWithoutArgs([this] {
            auto tempFuture = std::promise<bool>();
            m_releaseChannelPromise.swap(tempFuture);
            m_releaseChannelPromise.set_value(true);
            return m_releaseChannelPromise.get_future();
        }));
        EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS));
        m_engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
            m_mockPlatformInterface,
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_mockContextManager,
            m_mockExceptionSender,
            m_mockMessageSender,
            m_mockFocusManager,
            m_mockAuthDelegate,
            m_deviceInfo,
            m_alexaEndpointInterface
        );
    }
    void TearDown() override {
        m_engineImpl->shutdown();
        if( alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::isInitialized() ) {
            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
        }
    }
    
    std::promise<bool> m_releaseChannelPromise;
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> m_engineImpl;
    std::shared_ptr<aace::phoneCallController::PhoneCallController> m_mockPlatformInterface;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>> m_mockContextManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>> m_mockMessageSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>> m_mockFocusManager;
    std::shared_ptr<alexa::AlexaMockComponentFactory> m_alexaMockFactory;
    std::shared_ptr<testing::StrictMock<MockAuthDelegateInterface>> m_mockAuthDelegate;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> m_alexaEndpointInterface;
};

TEST_F( PhoneCallControllerEngineImplTest, create ) {
    EXPECT_NE(nullptr, m_engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullPlatform ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl; 
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        nullptr, m_alexaMockFactory->getEndpointBuilderMock(), m_mockContextManager, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager, m_mockAuthDelegate, m_deviceInfo, m_alexaEndpointInterface );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullCapabilitiesDelegate ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, nullptr, m_mockContextManager, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager, m_mockAuthDelegate, m_deviceInfo, m_alexaEndpointInterface );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullContextManager ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_alexaMockFactory->getEndpointBuilderMock(), nullptr, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager, m_mockAuthDelegate, m_deviceInfo, m_alexaEndpointInterface );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullExceptionSender ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_alexaMockFactory->getEndpointBuilderMock(), m_mockContextManager, nullptr, m_mockMessageSender, m_mockFocusManager, m_mockAuthDelegate, m_deviceInfo, m_alexaEndpointInterface );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullMessageSender ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_alexaMockFactory->getEndpointBuilderMock(), m_mockContextManager, m_mockExceptionSender, nullptr, m_mockFocusManager, m_mockAuthDelegate, m_deviceInfo, m_alexaEndpointInterface );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullFocusManager ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_alexaMockFactory->getEndpointBuilderMock(), m_mockContextManager, m_mockExceptionSender, m_mockMessageSender, nullptr, m_mockAuthDelegate, m_deviceInfo, m_alexaEndpointInterface );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullAuthDelegate ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_alexaMockFactory->getEndpointBuilderMock(), m_mockContextManager, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager, nullptr, m_deviceInfo, m_alexaEndpointInterface );
    EXPECT_EQ(nullptr, engineImpl);
}

TEST_F( PhoneCallControllerEngineImplTest, createWithNullDeviceInfo ) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerEngineImpl> engineImpl;
    engineImpl = aace::engine::phoneCallController::PhoneCallControllerEngineImpl::create(
        m_mockPlatformInterface, m_alexaMockFactory->getEndpointBuilderMock(), m_mockContextManager, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager, m_mockAuthDelegate, nullptr, m_alexaEndpointInterface );
    EXPECT_EQ(nullptr, engineImpl);
}

} // aace::test::unit
} // aace::test
} // aace
