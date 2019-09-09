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

#include <functional>
#include <istream>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>

#include <AACE/Test/Alexa/AlexaTestHelper.h>
#include <AACE/Test/Audio/MockAudioManagerInterface.h>
#include <AACE/Test/Audio/MockAudioOutputChannelInterface.h>
#include <AACE/Test/Alexa/MockSpeechRecognizer.h>

#include <AACE/Engine/Alexa/SpeechRecognizerEngineImpl.h>

using namespace aace::test::alexa;
using namespace aace::test::audio;

class SpeechRecognizerEngineImplTest : public ::testing::Test {
public:
    void SetUp() override
    {
        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();
        
        // initialize the avs device SDK
        ASSERT_TRUE( alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize( { AlexaTestHelper::getAVSConfig() } ) ) << "Initialize AVS Device SDK Failed!";
        
        // initialized succeeded
        m_initialized = true;
    }

    void TearDown() override
    {
        if( m_initialized )
        {
            m_alexaMockFactory->shutdown();
        
            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
            
            m_initialized = false;
        }
    }
    
protected:
    void configure()
    {
        if( m_configured == false )
        {
            EXPECT_CALL(*m_alexaMockFactory->getAudioManagerMock(),openAudioInputChannel("SpeechRecognizer",aace::audio::AudioInputProvider::AudioInputType::VOICE))
                .WillOnce(testing::Return(m_alexaMockFactory->getAudioInputChannelMock()));
            EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
            EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());
            EXPECT_CALL(*m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),registerCapability(testing::_)).WillOnce(testing::Return(true));
            
            m_configured = true;
        }
    }
    
    std::shared_ptr<aace::engine::alexa::SpeechRecognizerEngineImpl> createSpeechRecognizerEngineImpl()
    {
        if( m_configured == false ) {
            configure();
        }
        
        auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
            m_alexaMockFactory->getSpeechRecognizerMock(),
            createAudioFormat(),
            m_alexaMockFactory->getAudioManagerMock(),
            m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock(),
            m_alexaMockFactory->getFocusManagerInterfaceMock(),
            m_alexaMockFactory->getDialogUXStateAggregatorMock(),
            m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getUserInactivityMonitorMock() );
        
        return speechRecognizerEngineImpl;
    }
    
    alexaClientSDK::avsCommon::utils::AudioFormat createAudioFormat()
    {
        alexaClientSDK::avsCommon::utils::AudioFormat format;
        
        format.sampleRateHz = 16000;
        format.sampleSizeInBits = 16;
        format.numChannels = 1;
        format.endianness = alexaClientSDK::avsCommon::utils::AudioFormat::Endianness::LITTLE;
        format.encoding = alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::LPCM;
    
        return format;
    }

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

TEST_F(SpeechRecognizerEngineImplTest, create)
{
    auto speechRecognizerEngineImpl = createSpeechRecognizerEngineImpl();
    ASSERT_NE(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be not null";
    
    speechRecognizerEngineImpl->shutdown();
}

TEST_F(SpeechRecognizerEngineImplTest,createWithPlatformInterfaceAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        nullptr,
        createAudioFormat(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getUserInactivityMonitorMock() );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechRecognizerEngineImplTest, createWithAudioManagerInterfaceAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_alexaMockFactory->getSpeechRecognizerMock(),
        createAudioFormat(),
        nullptr,
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getUserInactivityMonitorMock() );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechRecognizerEngineImplTest,createWithDirectiveSequencerAsNull)
{
    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_alexaMockFactory->getSpeechRecognizerMock(),
        createAudioFormat(),
        m_alexaMockFactory->getAudioManagerMock(),
        nullptr,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getUserInactivityMonitorMock() );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechRecognizerEngineImplTest,createWithMessageSenderAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_alexaMockFactory->getSpeechRecognizerMock(),
        createAudioFormat(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getUserInactivityMonitorMock() );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechRecognizerEngineImplTest,createWithContextManagerAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_alexaMockFactory->getSpeechRecognizerMock(),
        createAudioFormat(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getUserInactivityMonitorMock() );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechRecognizerEngineImplTest,createWithFocusManagerAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_alexaMockFactory->getSpeechRecognizerMock(),
        createAudioFormat(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getUserInactivityMonitorMock() );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechRecognizerEngineImplTest,createWithDialogUXStateAggregatorAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_alexaMockFactory->getSpeechRecognizerMock(),
        createAudioFormat(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getUserInactivityMonitorMock() );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechRecognizerEngineImplTest, createWithCapabilitiesDelegateAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_alexaMockFactory->getSpeechRecognizerMock(),
        createAudioFormat(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        nullptr,
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getUserInactivityMonitorMock() );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechRecognizerEngineImplTest,createWithExceptionSenderAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_alexaMockFactory->getSpeechRecognizerMock(),
        createAudioFormat(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getUserInactivityMonitorMock() );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechRecognizerEngineImplTest,createWithUserInactivityMonitorAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto speechRecognizerEngineImpl = aace::engine::alexa::SpeechRecognizerEngineImpl::create(
        m_alexaMockFactory->getSpeechRecognizerMock(),
        createAudioFormat(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        nullptr );
    
    ASSERT_EQ(speechRecognizerEngineImpl,nullptr) << "SpeechRecognizerEngineImpl pointer expected to be null";
}
