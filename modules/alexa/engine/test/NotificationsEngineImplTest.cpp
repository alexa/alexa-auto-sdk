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

#include "AACE/Engine/Alexa/NotificationsEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/Audio/NotificationsAudioFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include "include/MockSpeakerManager.h"
#include "include/MockCapabilitiesDelegateInterface.h"
#include "include/MockCustomerDataManager.h"
#include "include/MockMediaPlayer.h"
#include "include/MockSpeaker.h"

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon;

// clang-format off
static const std::string CAPABILITIES_CONFIG_JSON =
    "{"
    "    \"cblAuthDelegate\":{"
    "        \"databaseFilePath\":\"cblAuthDelegate.db\""
    "    },"
    "    \"deviceInfo\":{"
    "        \"deviceSerialNumber\":\"12345678\", "
    "        \"clientId\":\"DummyClientId\","
    "        \"productId\":\"DummyProductID\""
    "    },"
    "    \"capabilitiesDelegate\":{"
    "    },"
    "    \"miscDatabase\":{"
    "        \"databaseFilePath\":\"miscDatabase.db\""
    "    },"
    "    \"alertsCapabilityAgent\":{"
    "        \"databaseFilePath\":\"alerts.db\""
    "    },"
    "    \"settings\":{"
    "        \"databaseFilePath\":\"settings.db\","
    "        \"defaultAVSClientSettings\":{"
    "            \"locale\":\"en-US\""
    "        }"
    "    },"
    "    \"bluetooth\" : {"
    "        \"databaseFilePath\":\"bluetooth.db\""
    "    },"
    "    \"certifiedSender\":{"
    "        \"databaseFilePath\":\"certifiedSender.db\""
    "    },"
    "    \"notifications\":{ "
    "        \"databaseFilePath\":\"notifications.db\""
    "    },"
    "    \"sampleApp\":{"
    "    }"
    " }";

class MockNotificationsPlatformInteraface : public aace::alexa::Notifications {
 public:
     MockNotificationsPlatformInteraface( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, 
        std::shared_ptr<aace::alexa::Speaker> speaker ) : aace::alexa::Notifications(mediaPlayer,speaker) {
    }
    MOCK_METHOD1(setIndicator,void(IndicatorState state));
};

class MockNotificationsAudioFactoryInterface : public alexaClientSDK::avsCommon::sdkInterfaces::audio::NotificationsAudioFactoryInterface {
public:
    MOCK_CONST_METHOD0(notificationDefault, std::function<std::unique_ptr<std::istream>()>());
};

class NotificationsEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        }
        auto inString = std::shared_ptr<std::istringstream>(new std::istringstream(CAPABILITIES_CONFIG_JSON));
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize({inString});
        m_mediaPlayer = std::make_shared<aace::test::unit::MockMediaPlayer>();
        m_speaker = std::make_shared<aace::test::unit::MockSpeaker>();
        m_mockNotificationPlatformInterface = std::make_shared<aace::test::unit::MockNotificationsPlatformInteraface>(m_mediaPlayer,m_speaker);
        m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        m_mockContextManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockNotificationsAudioFactory = std::make_shared<testing::StrictMock<aace::test::unit::MockNotificationsAudioFactoryInterface>>();
        m_mockSpeakerManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>>();
        m_customerDataManager = std::make_shared<testing::StrictMock<MockCustomerDataManager>>();

        EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockContextManager, setStateProvider(testing::_, testing::_));
        EXPECT_CALL(*m_mockContextManager,setState(testing::_, testing::_,testing::_,testing::_)).Times(testing::Exactly(1));
        EXPECT_CALL(*m_mockNotificationPlatformInterface, setIndicator(aace::alexa::Notifications::IndicatorState::OFF));
        EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
        m_notificationEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
            m_mockNotificationPlatformInterface, m_mockDirectiveSequencer,m_mockContextManager, 
            m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockNotificationsAudioFactory,
            m_mockSpeakerManager, m_customerDataManager);

        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        }
    }

    void TearDown() override {
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        }
        if( m_mockDirectiveSequencer  != nullptr ) {
            EXPECT_CALL(*m_mockDirectiveSequencer, doShutdown()).Times(1);
            m_mockDirectiveSequencer->shutdown();
        }
        if( m_notificationEngineImpl  != nullptr ) {
            EXPECT_CALL(*m_mockContextManager, setStateProvider(testing::_, testing::_));
            m_notificationEngineImpl->shutdown();
        }
        if (avsCommon::avs::initialization::AlexaClientSDKInit::isInitialized()) {
            avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
        }
        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> m_notificationEngineImpl;
    std::shared_ptr<aace::alexa::MediaPlayer> m_mediaPlayer;
    std::shared_ptr<aace::alexa::Speaker> m_speaker;
    std::shared_ptr<aace::test::unit::MockNotificationsPlatformInteraface> m_mockNotificationPlatformInterface;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>> m_mockContextManager;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>> m_mockCapabilitiesDelegate;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockNotificationsAudioFactoryInterface>> m_mockNotificationsAudioFactory;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>> m_mockSpeakerManager;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCustomerDataManager>> m_customerDataManager;
};

/**
 * Test create() with valid parameters
 */
TEST_F( NotificationsEngineImplTest, create ) {
    EXPECT_NE(nullptr, m_notificationEngineImpl) << "NotificationEngineImpl pointer is null";
}

/**
 * Test create() with null parameters
 */
TEST_F(NotificationsEngineImplTest, createWithNotificationPlatformInterfaceAsNull) {
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> notificationEngineImplTemp;
    
    notificationEngineImplTemp = aace::engine::alexa::NotificationsEngineImpl::create(
        nullptr, m_mockDirectiveSequencer, m_mockContextManager,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockNotificationsAudioFactory,
        m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, notificationEngineImplTemp) << "NotificationEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(NotificationsEngineImplTest, createWithDirectiveSequencerAsNull) {
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> notificationEngineImplTemp;
    
    notificationEngineImplTemp = aace::engine::alexa::NotificationsEngineImpl::create(
        m_mockNotificationPlatformInterface, nullptr, m_mockContextManager,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockNotificationsAudioFactory,
        m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, notificationEngineImplTemp) << "NotificationEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(NotificationsEngineImplTest, createWithContextManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> notificationEngineImplTemp;

    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    notificationEngineImplTemp = aace::engine::alexa::NotificationsEngineImpl::create(
        m_mockNotificationPlatformInterface, m_mockDirectiveSequencer, nullptr,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockNotificationsAudioFactory,
        m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, notificationEngineImplTemp) << "NotificationEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(NotificationsEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> notificationEngineImplTemp;
    
    notificationEngineImplTemp = aace::engine::alexa::NotificationsEngineImpl::create(
        m_mockNotificationPlatformInterface, m_mockDirectiveSequencer, m_mockContextManager,
        nullptr, m_mockExceptionSender, m_mockNotificationsAudioFactory,
        m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, notificationEngineImplTemp) << "NotificationEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(NotificationsEngineImplTest, createWithExceptionSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> notificationEngineImplTemp;

    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    notificationEngineImplTemp = aace::engine::alexa::NotificationsEngineImpl::create(
        m_mockNotificationPlatformInterface, m_mockDirectiveSequencer, m_mockContextManager,
        m_mockCapabilitiesDelegate, nullptr, m_mockNotificationsAudioFactory,
        m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, notificationEngineImplTemp) << "NotificationEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(NotificationsEngineImplTest, createWithNotificationsAudioFactoryAsNull) {
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> notificationEngineImplTemp;

    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    notificationEngineImplTemp = aace::engine::alexa::NotificationsEngineImpl::create(
        m_mockNotificationPlatformInterface, m_mockDirectiveSequencer, m_mockContextManager,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, nullptr,
        m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, notificationEngineImplTemp) << "NotificationEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(NotificationsEngineImplTest, createWithSpeakerManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> notificationEngineImplTemp;
    
    notificationEngineImplTemp = aace::engine::alexa::NotificationsEngineImpl::create(
        m_mockNotificationPlatformInterface, m_mockDirectiveSequencer, m_mockContextManager,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockNotificationsAudioFactory,
        nullptr, m_customerDataManager);
    EXPECT_EQ(nullptr, notificationEngineImplTemp) << "NotificationEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(NotificationsEngineImplTest, createWithcustomerDataManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> notificationEngineImplTemp;

    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    notificationEngineImplTemp = aace::engine::alexa::NotificationsEngineImpl::create(
        m_mockNotificationPlatformInterface, m_mockDirectiveSequencer, m_mockContextManager,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockNotificationsAudioFactory,
        m_mockSpeakerManager, nullptr);
    EXPECT_EQ(nullptr, notificationEngineImplTemp) << "NotificationEngineImpl pointer to be null";
}

/**
 * Test verify the SetIndicator values.
 */
TEST_F(NotificationsEngineImplTest, verifySetIndicatorForON) {
    EXPECT_CALL(*m_mockNotificationPlatformInterface, setIndicator(aace::alexa::Notifications::IndicatorState::ON));
        m_notificationEngineImpl->onSetIndicator(alexaClientSDK::avsCommon::avs::IndicatorState::ON);
}

/**
 * Test verify the SetIndicator values.
 */
TEST_F(NotificationsEngineImplTest, verifySetIndicatorForUNKNOWN) {
    EXPECT_CALL(*m_mockNotificationPlatformInterface, setIndicator(aace::alexa::Notifications::IndicatorState::UNKNOWN));
    m_notificationEngineImpl->onSetIndicator(alexaClientSDK::avsCommon::avs::IndicatorState::UNDEFINED);
}

}  // namespace unit
}  // namespace test
}  // namespace aace

