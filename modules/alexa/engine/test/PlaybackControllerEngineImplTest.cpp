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

#include "AACE/Engine/Alexa/PlaybackControllerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include "include/MockCapabilitiesDelegateInterface.h"

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon;


class MockPlaybackControllerPlatformInteraface : public aace::alexa::PlaybackController {
public:

};


class PlaybackControllerEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        }

        m_mockPlaybackControllerPlatformInterface = std::make_shared<aace::test::unit::MockPlaybackControllerPlatformInteraface>();
        m_mockMessageSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>>();
        m_mockContextManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>>();

        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));

        m_playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
            m_mockPlaybackControllerPlatformInterface, m_mockMessageSender, m_mockContextManager,
            m_mockCapabilitiesDelegate);

        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        }
    }

    void TearDown() override {
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        }

        if( m_playbackControllerEngineImpl  != nullptr ) {
            m_playbackControllerEngineImpl->shutdown();
        }
       
        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::PlaybackControllerEngineImpl> m_playbackControllerEngineImpl;

    std::shared_ptr<aace::test::unit::MockPlaybackControllerPlatformInteraface> m_mockPlaybackControllerPlatformInterface;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>> m_mockMessageSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>> m_mockContextManager;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>> m_mockCapabilitiesDelegate;
};

/**
 * Test create() with valid parameters
 */
TEST_F( PlaybackControllerEngineImplTest, create ) {
    EXPECT_NE(nullptr, m_playbackControllerEngineImpl) << "PlaybackControllerEngineImpl pointer is null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(PlaybackControllerEngineImplTest, createWithPlaybackControllerAsNull)
{
    std::shared_ptr<aace::engine::alexa::PlaybackControllerEngineImpl> playbackControllerEngineImplTemp;
    m_playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
        nullptr, m_mockMessageSender, m_mockContextManager,
        m_mockCapabilitiesDelegate);
    EXPECT_EQ(nullptr, playbackControllerEngineImplTemp) << "PlaybackControllerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(PlaybackControllerEngineImplTest, createWithMessageSenderAsNull)
{
    std::shared_ptr<aace::engine::alexa::PlaybackControllerEngineImpl> playbackControllerEngineImplTemp;
    m_playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
        m_mockPlaybackControllerPlatformInterface, nullptr, m_mockContextManager,
        m_mockCapabilitiesDelegate);
    EXPECT_EQ(nullptr, playbackControllerEngineImplTemp) << "PlaybackControllerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(PlaybackControllerEngineImplTest, createWithContextManagerAsNull)
{
    std::shared_ptr<aace::engine::alexa::PlaybackControllerEngineImpl> playbackControllerEngineImplTemp;
    m_playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
        m_mockPlaybackControllerPlatformInterface, m_mockMessageSender, nullptr,
        m_mockCapabilitiesDelegate);
    EXPECT_EQ(nullptr, playbackControllerEngineImplTemp) << "PlaybackControllerEngineImpl should be null";
}

/**
 * Test create() with nullptr parameters
 */
TEST_F(PlaybackControllerEngineImplTest, createWithCapabilitiesDelegateAsNull)
{
    std::shared_ptr<aace::engine::alexa::PlaybackControllerEngineImpl> playbackControllerEngineImplTemp;
    m_playbackControllerEngineImpl = aace::engine::alexa::PlaybackControllerEngineImpl::create(
        m_mockPlaybackControllerPlatformInterface, m_mockMessageSender, m_mockContextManager,
        nullptr);
    EXPECT_EQ(nullptr, playbackControllerEngineImplTemp) << "PlaybackControllerEngineImpl should be null";
}

}  // namespace unit
}  // namespace test
}  // namespace aace

