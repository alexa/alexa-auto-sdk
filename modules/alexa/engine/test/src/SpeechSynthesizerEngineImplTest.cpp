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

#include <functional>
#include <istream>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>

#include <AACE/Test/Alexa/AlexaTestHelper.h>
#include <AACE/Test/Audio/MockAudioManagerInterface.h>
#include <AACE/Test/Audio/MockAudioOutputChannelInterface.h>
#include <AACE/Test/Alexa/MockSpeechSynthesizer.h>

#include <AACE/Engine/Alexa/SpeechSynthesizerEngineImpl.h>

using namespace aace::test::alexa;
using namespace aace::test::audio;

class SpeechSynthesizerEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();

        // initialize the avs device SDK
        ASSERT_TRUE(alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize(
            {AlexaTestHelper::getAVSConfig()}))
            << "Initialize AVS Device SDK Failed!";

        // initialized succeeded
        m_initialized = true;
    }

    void TearDown() override {
        if (m_initialized) {
            m_alexaMockFactory->shutdown();

            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();

            m_initialized = false;
        }
    }

protected:
    void configure() {
        if (m_configured == false) {
            EXPECT_CALL(
                *m_alexaMockFactory->getAudioManagerMock(),
                openAudioOutputChannel("SpeechSynthesizer", aace::audio::AudioOutputProvider::AudioOutputType::TTS))
                .WillOnce(testing::Return(m_alexaMockFactory->getAudioOutputChannelMock()));
            EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());
            EXPECT_CALL(*m_alexaMockFactory->getContextManagerInterfaceMock(), setStateProvider(testing::_, testing::_))
                .Times(testing::AtLeast(1));
            EXPECT_CALL(
                *m_alexaMockFactory->getSpeakerManagerInterfaceMock(), getSpeakerSettings(testing::_, testing::_))
                .WillRepeatedly(
                    testing::Invoke([](alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type,
                                       alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings*) {
                        std::promise<bool> promise;
                        promise.set_value(true);
                        return promise.get_future();
                    }));

            m_configured = true;
        }
    }

    std::shared_ptr<aace::engine::alexa::SpeechSynthesizerEngineImpl> createSpeechSynthesizerEngineImpl() {
        if (m_configured == false) {
            configure();
        }

        auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
            m_alexaMockFactory->getSpeechSynthesizerMock(),
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_alexaMockFactory->getAudioManagerMock(),
            m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            m_alexaMockFactory->getFocusManagerInterfaceMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock(),
            m_alexaMockFactory->getAttachmentManagerMock(),
            m_alexaMockFactory->getDialogUXStateAggregatorMock(),
            m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
            m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

        return speechSynthesizerEngineImpl;
    }

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

TEST_F(SpeechSynthesizerEngineImplTest, create) {
    auto speechSynthesizerEngineImpl = createSpeechSynthesizerEngineImpl();
    ASSERT_NE(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be not null";

    speechSynthesizerEngineImpl->shutdown();
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithPlatformInterfaceAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithAudioManagerInterfaceAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        nullptr,
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithDirectiveSequencerAsNull) {
    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        nullptr,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithMessageSenderAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithFocusManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithContextManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithAttachmentManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithDialogUXStateAggregatorAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        nullptr,
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        nullptr,
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithSpeakerManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}

TEST_F(SpeechSynthesizerEngineImplTest, createWithExceptionSenderAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto speechSynthesizerEngineImpl = aace::engine::alexa::SpeechSynthesizerEngineImpl::create(
        m_alexaMockFactory->getSpeechSynthesizerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        nullptr);

    ASSERT_EQ(speechSynthesizerEngineImpl, nullptr) << "SpeechSynthesizerEngineImpl pointer expected to be null";
}
