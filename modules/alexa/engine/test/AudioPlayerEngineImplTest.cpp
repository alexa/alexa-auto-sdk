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

#include "AACE/Engine/Alexa/AudioPlayerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/test/MockAVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>
#include <AVSCommon/SDKInterfaces/test/MockPlaybackRouter.h>
#include "include/MockSpeakerManager.h"
#include "include/MockAttachmentManager.h"
#include "include/MockCapabilitiesDelegateInterface.h"
#include "include/MockConnectionStatusObserver.h"
#include "include/MockMediaPlayer.h"
#include "include/MockMessageRouter.h"
#include "include/MockSpeaker.h"

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon;
using namespace alexaClientSDK::acl;

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
    
class MockAudioPlayer : public aace::alexa::AudioPlayer {
public:
    MockAudioPlayer( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, 
        std::shared_ptr<aace::alexa::Speaker> speaker ) : aace::alexa::AudioPlayer(mediaPlayer,speaker) {
    }
    MOCK_METHOD1(playerActivityChanged, 
        void(PlayerActivity state));
};

class AudioPlayerEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }
        auto inString = std::shared_ptr<std::istringstream>(new std::istringstream(CAPABILITIES_CONFIG_JSON));
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize({inString});
        
        m_mediaPlayer = std::make_shared<aace::test::unit::MockMediaPlayer>();
        m_speaker = std::make_shared<aace::test::unit::MockSpeaker>();

        m_mockAudioPlayerPlatformInterface = std::make_shared<aace::test::unit::MockAudioPlayer>(m_mediaPlayer,m_speaker);

        m_mockMessageRouter = std::make_shared<testing::StrictMock<aace::test::unit::MockMessageRouter>>();
        EXPECT_CALL(*m_mockMessageRouter, setObserver(testing::_)).Times(1);
        EXPECT_CALL(*m_mockMessageRouter, enable()).Times(1);
        m_mockConnectionManager = AVSConnectionManager::create(
            m_mockMessageRouter,
            true,
            std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>(),
            std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface>>());
     
        m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        m_mockFocusManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>>();
        m_mockContextManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockAttachmentManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager>>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>>();
        m_mockSpeakerManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockPlaybackRouter = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockPlaybackRouter>>();

        EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockContextManager, setStateProvider(testing::_, testing::_));
        EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
        m_audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
            m_mockAudioPlayerPlatformInterface, m_mockDirectiveSequencer, m_mockConnectionManager,
            m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockCapabilitiesDelegate, m_mockSpeakerManager,
            m_mockExceptionSender, m_mockPlaybackRouter );
        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    void TearDown() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }
        
        if( m_mockDirectiveSequencer  != nullptr ) {
            EXPECT_CALL(*m_mockDirectiveSequencer, doShutdown()).Times(1);
            m_mockDirectiveSequencer->shutdown();
        }
        if( m_mockMessageRouter != nullptr ) {
            EXPECT_CALL(*m_mockMessageRouter, doShutdown()).Times(1);
            m_mockMessageRouter->shutdown();
        }
        if( m_mockConnectionManager != nullptr ) {
            EXPECT_CALL(*m_mockMessageRouter, disable()).Times(1);
            m_mockConnectionManager->shutdown();
        }   
        if( m_audioPlayerEngineImpl != nullptr ) {
            EXPECT_CALL(*m_mockContextManager, setStateProvider(testing::_, testing::_));
            m_audioPlayerEngineImpl->shutdown();
        }
        if (avsCommon::avs::initialization::AlexaClientSDKInit::isInitialized()) {
            avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
        }

        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    
    }

    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> m_audioPlayerEngineImpl;
    
    std::shared_ptr<aace::alexa::MediaPlayer> m_mediaPlayer;
    std::shared_ptr<aace::alexa::Speaker> m_speaker;
    std::shared_ptr<aace::test::unit::MockAudioPlayer> m_mockAudioPlayerPlatformInterface;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_mockConnectionManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_mockFocusManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>> m_mockContextManager;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager> m_mockAttachmentManager;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>> m_mockCapabilitiesDelegate;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>> m_mockSpeakerManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;
    std::shared_ptr<MockMessageRouter> m_mockMessageRouter;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> m_mockPlaybackRouter;


};

/**
 * Test create() with valid parameters
 */
TEST_F( AudioPlayerEngineImplTest, create ) {
    ASSERT_NE(nullptr, m_audioPlayerEngineImpl) << "AudioPlayerEngineImpl pointer is null";
}

/**
 * Test create() with null parameters
 */
TEST_F(AudioPlayerEngineImplTest, createWithPlayerInterfaceAsNull) {
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;
    
    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        nullptr, m_mockDirectiveSequencer, m_mockConnectionManager,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockCapabilitiesDelegate, m_mockSpeakerManager,
        m_mockExceptionSender, m_mockPlaybackRouter );
    EXPECT_EQ(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(AudioPlayerEngineImplTest, createWithDirectiveSequencerAsNull) {
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;

    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_mockAudioPlayerPlatformInterface, nullptr, m_mockConnectionManager,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockCapabilitiesDelegate, m_mockSpeakerManager,
        m_mockExceptionSender, m_mockPlaybackRouter );
    EXPECT_EQ(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(AudioPlayerEngineImplTest, createWithConnectionManagerNull){
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;

    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_mockAudioPlayerPlatformInterface, m_mockDirectiveSequencer, nullptr,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockCapabilitiesDelegate, m_mockSpeakerManager,
        m_mockExceptionSender, m_mockPlaybackRouter );
    EXPECT_EQ(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(AudioPlayerEngineImplTest, createWithFocusManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;

    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_mockAudioPlayerPlatformInterface, m_mockDirectiveSequencer, m_mockConnectionManager,
        nullptr, m_mockContextManager, m_mockAttachmentManager, m_mockCapabilitiesDelegate, m_mockSpeakerManager,
        m_mockExceptionSender, m_mockPlaybackRouter );
    EXPECT_EQ(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(AudioPlayerEngineImplTest, createWithContextManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;

    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_mockAudioPlayerPlatformInterface, m_mockDirectiveSequencer, m_mockConnectionManager,
        m_mockFocusManager, nullptr, m_mockAttachmentManager, m_mockCapabilitiesDelegate, m_mockSpeakerManager,
        m_mockExceptionSender, m_mockPlaybackRouter );
    EXPECT_EQ(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(AudioPlayerEngineImplTest, createWithAttachmentManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;

    EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockContextManager, setStateProvider(testing::_, testing::_));
    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_mockAudioPlayerPlatformInterface, m_mockDirectiveSequencer, m_mockConnectionManager,
        m_mockFocusManager, m_mockContextManager, nullptr, m_mockCapabilitiesDelegate, m_mockSpeakerManager,
        m_mockExceptionSender, m_mockPlaybackRouter );
    EXPECT_NE(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
    if (audioPlayerEngineImplTemp) {
        EXPECT_CALL(*m_mockContextManager, setStateProvider(testing::_, testing::_));
        audioPlayerEngineImplTemp->shutdown();
    }
}

/**
 * Test create() with null parameters
 */
TEST_F(AudioPlayerEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;

    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_mockAudioPlayerPlatformInterface, m_mockDirectiveSequencer, m_mockConnectionManager,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, nullptr, m_mockSpeakerManager,
        m_mockExceptionSender, m_mockPlaybackRouter );
    EXPECT_EQ(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(AudioPlayerEngineImplTest, createWithSpeakerManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;

    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_mockAudioPlayerPlatformInterface, m_mockDirectiveSequencer, m_mockConnectionManager,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockCapabilitiesDelegate, nullptr,
        m_mockExceptionSender, m_mockPlaybackRouter );
    EXPECT_EQ(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
TEST_F(AudioPlayerEngineImplTest, createWithExceptionSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;

    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_mockAudioPlayerPlatformInterface, m_mockDirectiveSequencer, m_mockConnectionManager,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockCapabilitiesDelegate, m_mockSpeakerManager,
        nullptr, m_mockPlaybackRouter );
    EXPECT_EQ(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
}

/**
 * Test create() with null parameters
 */
    TEST_F(AudioPlayerEngineImplTest, createWithPlayBackRouterAsNull) {
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> audioPlayerEngineImplTemp;

    EXPECT_CALL(*m_mockSpeakerManager, addSpeaker(testing::_));
    audioPlayerEngineImplTemp = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_mockAudioPlayerPlatformInterface, m_mockDirectiveSequencer, m_mockConnectionManager,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockCapabilitiesDelegate, m_mockSpeakerManager,
        m_mockExceptionSender, nullptr );
    EXPECT_EQ(nullptr, audioPlayerEngineImplTemp) << "AudioPlayerEngineImpl pointer to be null";
}

}  // namespace unit
}  // namespace test
}  // namespace aace

