/*
 * Copyright 2018-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Test/Unit/Alexa/AlexaTestHelper.h>
#include <AACE/Test/Unit/Audio/MockAudioManagerInterface.h>
#include <AACE/Test/Unit/Audio/MockAudioOutputChannelInterface.h>

#include <AACE/Engine/Alexa/AudioPlayerEngineImpl.h>
#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>

using namespace aace::test::unit::alexa;
using namespace aace::test::unit::audio;

/// The "content" channel name for AudioActivityTracker.
static const std::string CONTENT_CHANNEL_NAME{"Content"};

/// The interface name for AudioPlayer in AudioActivityTracker.
static const std::string AUDIO_PLAYER_INTERFACE_NAME{"AudioPlayer"};

class AudioPlayerEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();

        // initialize the avs device SDK
        ASSERT_TRUE(alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize(
            {AlexaTestHelper::getAVSConfig()}))
            << "Initialize AVS Device SDK Failed!";

        EXPECT_CALL(
            *m_alexaMockFactory->getAudioManagerMock(),
            openAudioOutputChannel("AudioPlayer", aace::audio::AudioOutputProvider::AudioOutputType::MUSIC))
            .WillOnce(testing::Return(m_alexaMockFactory->getAudioOutputChannelMock()));
        EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());
        EXPECT_CALL(*m_alexaMockFactory->getContextManagerInterfaceMock(), setStateProvider(testing::_, testing::_))
            .Times(testing::AtLeast(1));
        EXPECT_CALL(*m_alexaMockFactory->getSpeakerManagerInterfaceMock(), getSpeakerSettings(testing::_, testing::_))
            .WillRepeatedly(
                testing::Invoke([](alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type,
                                   alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings*) {
                    std::promise<bool> promise;
                    promise.set_value(true);
                    return promise.get_future();
                }));

        m_engineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
            m_alexaMockFactory->getAudioPlayerMock(),
            m_alexaMockFactory->getAudioManagerMock(),
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            m_alexaMockFactory->getFocusManagerInterfaceMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock(),
            m_alexaMockFactory->getAttachmentManagerMock(),
            m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getPlaybackRouterMock(),
            m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
            m_alexaMockFactory->getMetricRecorder(),
            m_alexaMockFactory->getActivityTrackerInterfaceMock(),
            m_alexaMockFactory->getCustomerDataManagerMock());
        ASSERT_NE(m_engineImpl, nullptr);
    }

    void TearDown() override {
        m_alexaMockFactory->shutdown();

        m_engineImpl->shutdown();
        m_engineImpl.reset();

        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
    }

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;
    std::shared_ptr<aace::engine::alexa::AudioPlayerEngineImpl> m_engineImpl;
};

TEST_F(AudioPlayerEngineImplTest, createWithPlatformInterfaceAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithAudioManagerInterfaceAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        nullptr,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithDirectiveSequencerAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        nullptr,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithMessageSenderAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        nullptr,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithFocusManagerAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithContextManagerAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithAttachmentManagerAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithSpeakerManagerAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        nullptr,
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithExceptionSenderAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithPlaybackRouterAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithAudioPlayerObserverInterfaceAsNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        nullptr,
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithMetricRecorderNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithActivityTrackerNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        nullptr,
        m_alexaMockFactory->getCustomerDataManagerMock());

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

TEST_F(AudioPlayerEngineImplTest, createWithCustomerDataManagerNull) {
    auto audioPlayerEngineImpl = aace::engine::alexa::AudioPlayerEngineImpl::create(
        m_alexaMockFactory->getAudioPlayerMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getAttachmentManagerMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getPlaybackRouterMock(),
        m_alexaMockFactory->getAudioPlayerObserverInterfaceMock(),
        m_alexaMockFactory->getMetricRecorder(),
        m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        nullptr);

    ASSERT_EQ(audioPlayerEngineImpl, nullptr) << "AudioPlayerEngineImpl pointer expected to be null";
}

/**
 * Compare actual and expected @c alexaClientSDK::afml::Channel::State vectors for equality
 */
MATCHER_P(matchChannelState, expected, "") {
    EXPECT_EQ(arg.size(), expected.size());
    auto count = 0;
    for (auto& expectedChannel : expected) {
        auto& channel = arg[count];
        EXPECT_EQ(channel.name, expectedChannel.name);
        EXPECT_EQ(channel.interfaceName, expectedChannel.interfaceName);
        EXPECT_EQ(channel.focusState, expectedChannel.focusState);
        count++;
    }
    return true;
}

/**
 * Verifies that @c AudioPlayerEngineInterface::onSetAsForegroundActivity() sets AudioPlayer in foreground
 * content channel focus of AudioActivityTracker.
 */
TEST_F(AudioPlayerEngineImplTest, testSetAsForegroundActivityCallsActivityTracker) {
    alexaClientSDK::afml::Channel::State expectedState = alexaClientSDK::afml::Channel::State(CONTENT_CHANNEL_NAME);
    expectedState.focusState = alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND;
    expectedState.interfaceName = AUDIO_PLAYER_INTERFACE_NAME;
    expectedState.name = CONTENT_CHANNEL_NAME;
    std::vector<alexaClientSDK::afml::Channel::State> expectedStates = {expectedState};
    EXPECT_CALL(
        *m_alexaMockFactory->getActivityTrackerInterfaceMock(),
        notifyOfActivityUpdates(matchChannelState(expectedStates)))
        .Times(1);

    m_engineImpl->onSetAsForegroundActivity();
}