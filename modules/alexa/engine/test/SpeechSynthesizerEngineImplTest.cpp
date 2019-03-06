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

#include "AACE/Engine/Alexa/SpeechSynthesizerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include "include/MockSpeakerManager.h"
#include "include/MockAttachmentManager.h"
#include "include/MockCapabilitiesDelegateInterface.h"
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

class MockSpeechSynthesizerPlatformInterface : public aace::alexa::SpeechSynthesizer {
public:
    MockSpeechSynthesizerPlatformInterface(std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer,
        std::shared_ptr<aace::alexa::Speaker> speaker) : aace::alexa::SpeechSynthesizer(mediaPlayer, speaker){
    }
};

class SpeechSynthesizerEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }

        m_mediaPlayer = std::make_shared<aace::test::unit::MockMediaPlayer>();
        m_speaker = std::make_shared<aace::test::unit::MockSpeaker>();

        m_mockSpeechSynthesizerPlatformInterface = std::make_shared<testing::StrictMock<MockSpeechSynthesizerPlatformInterface>>(
            m_mediaPlayer, m_speaker);

        m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        m_mockMessageSender = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>();
        m_mockFocusManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>>();
        m_mockContextManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockAttachmentManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager>>();
        m_mockDialogUXStateAggregator = std::make_shared<avsCommon::avs::DialogUXStateAggregator>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>>();
        m_mockSpeakerManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();

        EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockContextManager, setStateProvider(testing::_, testing::_));
        m_speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
            m_mockSpeechSynthesizerPlatformInterface, m_mockDirectiveSequencer, m_mockMessageSender,
            m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockDialogUXStateAggregator,
            m_mockCapabilitiesDelegate, m_mockSpeakerManager, m_mockExceptionSender);

        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    void TearDown() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }
        if( m_mockDirectiveSequencer != nullptr ) {
            EXPECT_CALL(*m_mockDirectiveSequencer, doShutdown()).Times(1);
            m_mockDirectiveSequencer->shutdown();
        }
        if( m_speechSynthesizerEngineImpl != nullptr ) {
            m_speechSynthesizerEngineImpl->shutdown();
        }

        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> m_speechSynthesizerEngineImpl;
    std::shared_ptr<aace::test::unit::MockSpeechSynthesizerPlatformInterface> m_mockSpeechSynthesizerPlatformInterface;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender> m_mockMessageSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>> m_mockFocusManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>> m_mockContextManager;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager> m_mockAttachmentManager;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> m_mockDialogUXStateAggregator;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>> m_mockCapabilitiesDelegate;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>> m_mockSpeakerManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> m_mockExceptionSender;
    std::shared_ptr<aace::alexa::MediaPlayer> m_mediaPlayer;
    std::shared_ptr<aace::alexa::Speaker> m_speaker;
};

/**
 * Test create() with valid parameters
 */
TEST_F( SpeechSynthesizerEngineImplTest, create ) {
    EXPECT_NE(nullptr, m_speechSynthesizerEngineImpl) << "SpeechSynthesizerEngineImpl pointer is null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithSpeechSynthesizerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;
    
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        nullptr, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockSpeakerManager, m_mockExceptionSender);
    EXPECT_EQ(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithDirectiveSequencerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;
    
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_mockSpeechSynthesizerPlatformInterface, nullptr, m_mockMessageSender,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockSpeakerManager, m_mockExceptionSender);
    EXPECT_EQ(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithMessageSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;
    
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_mockSpeechSynthesizerPlatformInterface, m_mockDirectiveSequencer, nullptr,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockSpeakerManager, m_mockExceptionSender);
    EXPECT_EQ(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithFocusManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;
    
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_mockSpeechSynthesizerPlatformInterface, m_mockDirectiveSequencer, m_mockMessageSender,
        nullptr, m_mockContextManager, m_mockAttachmentManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockSpeakerManager, m_mockExceptionSender);
    EXPECT_EQ(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithContextManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;
    
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_mockSpeechSynthesizerPlatformInterface, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockFocusManager, nullptr, m_mockAttachmentManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockSpeakerManager, m_mockExceptionSender);
    EXPECT_EQ(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithAttachmentManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;

    EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockContextManager, setStateProvider(testing::_, testing::_));
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_mockSpeechSynthesizerPlatformInterface, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockFocusManager, m_mockContextManager, nullptr, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockSpeakerManager, m_mockExceptionSender);
    EXPECT_NE(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl is not null";
    if( speechSynthesizerEngineImplTemp != nullptr ) {
        speechSynthesizerEngineImplTemp->shutdown();
    }
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithDialogUXStateAggregatorAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;
    
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_mockSpeechSynthesizerPlatformInterface, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, nullptr,
        m_mockCapabilitiesDelegate, m_mockSpeakerManager, m_mockExceptionSender);
    EXPECT_EQ(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;
    
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_mockSpeechSynthesizerPlatformInterface, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockDialogUXStateAggregator,
        nullptr, m_mockSpeakerManager, m_mockExceptionSender);
    EXPECT_EQ(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithSpeakerManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;
    
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_mockSpeechSynthesizerPlatformInterface, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, nullptr, m_mockExceptionSender);
    EXPECT_EQ(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechSynthesizerEngineImplTest, createWithExceptionSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> speechSynthesizerEngineImplTemp;
    
    speechSynthesizerEngineImplTemp = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_mockSpeechSynthesizerPlatformInterface, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockFocusManager, m_mockContextManager, m_mockAttachmentManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockSpeakerManager, nullptr);
    EXPECT_EQ(nullptr, speechSynthesizerEngineImplTemp) << "SpeechSynthesizerEngineImpl pointer should be null";
}


}  // namespace unit
}  // namespace test
}  // namespace aace

