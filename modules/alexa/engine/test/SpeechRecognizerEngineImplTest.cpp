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

#include "AACE/Engine/Alexa/SpeechRecognizerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include <AVSCommon/SDKInterfaces/test/MockUserInactivityMonitor.h>
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

static const avsCommon::utils::AudioFormat::Encoding COMPATIBLE_ENCODING =
    avsCommon::utils::AudioFormat::Encoding::LPCM;
static const avsCommon::utils::AudioFormat::Endianness COMPATIBLE_ENDIANNESS =
    avsCommon::utils::AudioFormat::Endianness::LITTLE;
static const unsigned int COMPATIBLE_SAMPLE_RATE = 16000;
static const unsigned int COMPATIBLE_SAMPLE_SIZE_IN_BITS = 16;
static const unsigned int COMPATIBLE_NUM_CHANNELS = 1;

class MockSpeechRecognizerPlatformInterface : public aace::alexa::SpeechRecognizer {
public:
    MockSpeechRecognizerPlatformInterface(bool wakewordDetectionEnabled) : 
        aace::alexa::SpeechRecognizer(wakewordDetectionEnabled){
    }
    MOCK_METHOD1(wakewordDetected,
                 bool(const std::string &wakeword));
    MOCK_METHOD0(endOfSpeechDetected,
                 void());
    MOCK_METHOD0(startAudioInput,
                 bool());
    MOCK_METHOD0(stopAudioInput,
                 bool());
};

class SpeechRecognizerEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }

        m_mockSpeechRecognizerPlatformInterface = std::make_shared<testing::StrictMock<MockSpeechRecognizerPlatformInterface>>(true);
        m_audioFormat.sampleRateHz = COMPATIBLE_SAMPLE_RATE;
        m_audioFormat.sampleSizeInBits = COMPATIBLE_SAMPLE_SIZE_IN_BITS;
        m_audioFormat.numChannels = COMPATIBLE_NUM_CHANNELS;
        m_audioFormat.endianness = COMPATIBLE_ENDIANNESS;
        m_audioFormat.encoding = COMPATIBLE_ENCODING;
        m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        m_mockMessageSender = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>();
        m_mockContextManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockFocusManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>>();
        m_mockDialogUXStateAggregator = std::make_shared<avsCommon::avs::DialogUXStateAggregator>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockUserInactivityMonitor = std::make_shared<avsCommon::sdkInterfaces::test::MockUserInactivityMonitor>();

        EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));

        m_speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
            m_mockSpeechRecognizerPlatformInterface, m_audioFormat, m_mockDirectiveSequencer, m_mockMessageSender,
            m_mockContextManager, m_mockFocusManager, m_mockDialogUXStateAggregator,
            m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockUserInactivityMonitor);

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
        if (m_speechRecognizerEngineImpl != nullptr ) {
            m_speechRecognizerEngineImpl->shutdown();
        }
        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> m_speechRecognizerEngineImpl;
    std::shared_ptr<aace::test::unit::MockSpeechRecognizerPlatformInterface> m_mockSpeechRecognizerPlatformInterface;
    alexaClientSDK::avsCommon::utils::AudioFormat m_audioFormat;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender> m_mockMessageSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>> m_mockContextManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>> m_mockFocusManager;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> m_mockDialogUXStateAggregator;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>> m_mockCapabilitiesDelegate;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> m_mockExceptionSender;
    std::shared_ptr<avsCommon::sdkInterfaces::test::MockUserInactivityMonitor> m_mockUserInactivityMonitor;
};

/**
 * Test create() with valid parameters
 */
TEST_F( SpeechRecognizerEngineImplTest, create ) {
    EXPECT_NE(nullptr, m_speechRecognizerEngineImpl) << "SpeechRecognizerEngineImpl pointer is null";
}
/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechRecognizerEngineImplTest, createSpeechRecognizerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> speechRecognizerEngineImplTemp;
    
    speechRecognizerEngineImplTemp = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        nullptr, m_audioFormat, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockContextManager, m_mockFocusManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockUserInactivityMonitor);
    EXPECT_EQ(nullptr, speechRecognizerEngineImplTemp) << "SpeechRecognizerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechRecognizerEngineImplTest, createDirectiveSequencerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> speechRecognizerEngineImplTemp;
    
    speechRecognizerEngineImplTemp = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_mockSpeechRecognizerPlatformInterface, m_audioFormat, nullptr, m_mockMessageSender,
        m_mockContextManager, m_mockFocusManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockUserInactivityMonitor);
    EXPECT_EQ(nullptr, speechRecognizerEngineImplTemp) << "SpeechRecognizerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechRecognizerEngineImplTest, createMessageSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> speechRecognizerEngineImplTemp;
    
    speechRecognizerEngineImplTemp = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_mockSpeechRecognizerPlatformInterface, m_audioFormat, m_mockDirectiveSequencer, nullptr,
        m_mockContextManager, m_mockFocusManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockUserInactivityMonitor);
    EXPECT_EQ(nullptr, speechRecognizerEngineImplTemp) << "SpeechRecognizerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechRecognizerEngineImplTest, createContextManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> speechRecognizerEngineImplTemp;
    
    speechRecognizerEngineImplTemp = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_mockSpeechRecognizerPlatformInterface, m_audioFormat, m_mockDirectiveSequencer, m_mockMessageSender,
        nullptr, m_mockFocusManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockUserInactivityMonitor);
    EXPECT_EQ(nullptr, speechRecognizerEngineImplTemp) << "SpeechRecognizerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechRecognizerEngineImplTest, createFocusManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> speechRecognizerEngineImplTemp;
    
    speechRecognizerEngineImplTemp = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_mockSpeechRecognizerPlatformInterface, m_audioFormat, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockContextManager, nullptr, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockUserInactivityMonitor);
    EXPECT_EQ(nullptr, speechRecognizerEngineImplTemp) << "SpeechRecognizerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechRecognizerEngineImplTest, createDialogUXStateAggregatorAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> speechRecognizerEngineImplTemp;

    speechRecognizerEngineImplTemp = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_mockSpeechRecognizerPlatformInterface, m_audioFormat, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockContextManager, m_mockFocusManager, nullptr,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, m_mockUserInactivityMonitor);
    EXPECT_EQ(nullptr, speechRecognizerEngineImplTemp) << "SpeechRecognizerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechRecognizerEngineImplTest, createCapabilitiesDelegateAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> speechRecognizerEngineImplTemp;

    speechRecognizerEngineImplTemp = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_mockSpeechRecognizerPlatformInterface, m_audioFormat, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockContextManager, m_mockFocusManager, m_mockDialogUXStateAggregator,
        nullptr, m_mockExceptionSender, m_mockUserInactivityMonitor);
    EXPECT_EQ(nullptr, speechRecognizerEngineImplTemp) << "SpeechRecognizerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechRecognizerEngineImplTest, createExceptionSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> speechRecognizerEngineImplTemp;

    speechRecognizerEngineImplTemp = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_mockSpeechRecognizerPlatformInterface, m_audioFormat, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockContextManager, m_mockFocusManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, nullptr, m_mockUserInactivityMonitor);
    EXPECT_EQ(nullptr, speechRecognizerEngineImplTemp) << "SpeechRecognizerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(SpeechRecognizerEngineImplTest, createUserInactivityMonitorAsNull) {
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> speechRecognizerEngineImplTemp;

    speechRecognizerEngineImplTemp = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_mockSpeechRecognizerPlatformInterface, m_audioFormat, m_mockDirectiveSequencer, m_mockMessageSender,
        m_mockContextManager, m_mockFocusManager, m_mockDialogUXStateAggregator,
        m_mockCapabilitiesDelegate, m_mockExceptionSender, nullptr);
    EXPECT_EQ(nullptr, speechRecognizerEngineImplTemp) << "SpeechRecognizerEngineImpl should be null";
}

}  // namespace unit
}  // namespace test
}  // namespace aace

