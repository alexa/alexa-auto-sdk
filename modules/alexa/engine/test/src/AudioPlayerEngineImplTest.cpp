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
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>

#include <AACE/Test/Alexa/AlexaTestHelper.h>
#include <AACE/Test/Audio/MockAudioManagerInterface.h>
#include <AACE/Test/Audio/MockAudioOutputChannelInterface.h>
#include <AACE/Test/Alexa/MockAlerts.h>

#include <AACE/Engine/Alexa/AudioPlayerEngineImpl.h>

#include <CertifiedSender/CertifiedSender.h>

using namespace aace::test::alexa;
using namespace aace::test::audio;
using ::testing::Return;

class AudioPlayerEngineImplTest : public ::testing::Test {
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
                openAudioOutputChannel("AudioPlayer", aace::audio::AudioOutputProvider::AudioOutputType::MUSIC))
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

    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> createAudioPlayerEngineImpl() {
        if (m_configured == false) {
            configure();
        }

        auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
            m_alexaMockFactory->getAudioPlayerMock(),
            m_alexaMockFactory->getAudioManagerMock(),
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            m_alexaMockFactory->getFocusManagerInterfaceMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock(),
            m_alexaMockFactory->getAttachmentManagerMock(),
            m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
            m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getPlaybackRouterMock(),
            m_alexaMockFactory->getCertifiedSenderMock(),
            m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
            m_alexaMockFactory->getAuthDelegateInterfaceMock());

        return audioPlayerEngineImpl;
    }

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

TEST_F(AudioPlayerEngineImplTest, create) {
    auto audioPlayerEngineImpl = createAudioPlayerEngineImpl();
    ASSERT_NE(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be not null";

    audioPlayerEngineImpl->shutdown();
}

TEST_F(AudioPlayerEngineImplTest, createWithPlatformInterfaceAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithAudioManagerInterfaceAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        nullptr,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithDirectiveSequencerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        nullptr,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithMessageSenderAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        nullptr,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithFocusManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithContextManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithAttachmentManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        nullptr,
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithSpeakerManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithExceptionSenderAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithPlaybackRouterAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithCertifiedSenderAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        nullptr,
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithAudioPlayerObserverInterfaceAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        nullptr,
        m_alexaMockFactory->getAuthDelegateInterfaceMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithAuthProviderNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        nullptr);

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}
