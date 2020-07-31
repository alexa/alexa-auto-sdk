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
#include <AACE/Engine/Alexa/PlaybackControllerEngineImpl.h>

using namespace aace::test::alexa;

class PlaybackControllerEngineImplTest : public ::testing::Test {
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
            m_configured = true;
        }
    }

    std::shared_ptr<aace::engine::alexa::PlaybackControllerEngineImpl> createPlaybackControllerEngineImpl() {
        if (m_configured == false) {
            configure();
        }

        auto playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
            m_alexaMockFactory->getPlaybackControllerMock(),
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock(),
            m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
            m_alexaMockFactory->getFocusManagerInterfaceMock());

        return playbackControllerEngineImpl;
    }

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

TEST_F(PlaybackControllerEngineImplTest, create) {
    auto playbackControllerEngineImpl = createPlaybackControllerEngineImpl();
    ASSERT_NE(playbackControllerEngineImpl, nullptr) << "PlaybackControllerEngineImpl pointer expected to be not null!";

    playbackControllerEngineImpl->shutdown();
}

TEST_F(PlaybackControllerEngineImplTest, createWithPlatformInterfaceAsNull) {
    auto playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(playbackControllerEngineImpl, nullptr) << "PlaybackControllerEngineImpl pointer expected to be null";
}

TEST_F(PlaybackControllerEngineImplTest, createWithMessageSenderAsNull) {
    auto playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
        m_alexaMockFactory->getPlaybackControllerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        nullptr,
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(playbackControllerEngineImpl, nullptr) << "PlaybackControllerEngineImpl pointer expected to be null";
}

TEST_F(PlaybackControllerEngineImplTest, createWithContextManagerAsNull) {
    auto playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
        m_alexaMockFactory->getPlaybackControllerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(playbackControllerEngineImpl, nullptr) << "PlaybackControllerEngineImpl pointer expected to be null";
}

TEST_F(PlaybackControllerEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    auto playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
        m_alexaMockFactory->getPlaybackControllerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(playbackControllerEngineImpl, nullptr) << "PlaybackControllerEngineImpl pointer expected to be null";
}

TEST_F(PlaybackControllerEngineImplTest, createWithFocusManagerAsNull) {
    auto playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
        m_alexaMockFactory->getPlaybackControllerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        nullptr);

    ASSERT_EQ(playbackControllerEngineImpl, nullptr) << "PlaybackControllerEngineImpl pointer expected to be null";
}
