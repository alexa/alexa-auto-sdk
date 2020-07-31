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
#include <AACE/Test/Alexa/MockNotifications.h>

#include <AACE/Engine/Alexa/NotificationsEngineImpl.h>

using namespace aace::test::alexa;
using namespace aace::test::audio;

class NotificationsEngineImplTest : public ::testing::Test {
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
                *m_alexaMockFactory->getNotificationsMock(),
                setIndicator(aace::alexa::Notifications::IndicatorState::OFF));
            EXPECT_CALL(
                *m_alexaMockFactory->getAudioManagerMock(),
                openAudioOutputChannel(
                    "Notifications", aace::audio::AudioOutputProvider::AudioOutputType::NOTIFICATION))
                .WillOnce(testing::Return(m_alexaMockFactory->getAudioOutputChannelMock()));
            EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());
            EXPECT_CALL(
                *m_alexaMockFactory->getContextManagerInterfaceMock(),
                setState(testing::_, testing::_, testing::_, testing::_));
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

    std::shared_ptr<aace::engine::alexa::NotificationsEngineImpl> createNotificationsEngineImpl() {
        if (m_configured == false) {
            configure();
        }

        auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
            m_alexaMockFactory->getNotificationsMock(),
            m_alexaMockFactory->getAudioManagerMock(),
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock(),
            m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getNotificationsAudioFactoryInterfaceMock(),
            m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
            m_alexaMockFactory->getCustomerDataManagerMock(),
            m_alexaMockFactory->getFocusManagerInterfaceMock());

        return notificationsEngineImpl;
    }

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

TEST_F(NotificationsEngineImplTest, create) {
    auto notificationsEngineImpl = createNotificationsEngineImpl();
    ASSERT_NE(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be not null";

    notificationsEngineImpl->shutdown();
}

TEST_F(NotificationsEngineImplTest, createWithPlatformInterfaceAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getNotificationsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(NotificationsEngineImplTest, createWithAudioManagerInterfaceAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
        m_alexaMockFactory->getNotificationsMock(),
        nullptr,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getNotificationsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(NotificationsEngineImplTest, createWithDirectiveSequencerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
        m_alexaMockFactory->getNotificationsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        nullptr,
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getNotificationsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(NotificationsEngineImplTest, createWithContextManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
        m_alexaMockFactory->getNotificationsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        nullptr,
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getNotificationsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(NotificationsEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
        m_alexaMockFactory->getNotificationsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getNotificationsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(NotificationsEngineImplTest, createWithExceptionSenderAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
        m_alexaMockFactory->getNotificationsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getNotificationsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(NotificationsEngineImplTest, createWithAudioFactoryAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
        m_alexaMockFactory->getNotificationsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(NotificationsEngineImplTest, createWithSpeakerManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
        m_alexaMockFactory->getNotificationsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getNotificationsAudioFactoryInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getCustomerDataManagerMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(NotificationsEngineImplTest, createWithCustomerDataManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto notificationsEngineImpl = aace::engine::alexa::NotificationsEngineImpl::create(
        m_alexaMockFactory->getNotificationsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getNotificationsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getFocusManagerInterfaceMock());

    ASSERT_EQ(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(NotificationsEngineImplTest, verifySetIndicatorForON) {
    EXPECT_CALL(
        *m_alexaMockFactory->getNotificationsMock(), setIndicator(aace::alexa::Notifications::IndicatorState::ON));

    auto notificationsEngineImpl = createNotificationsEngineImpl();
    ASSERT_NE(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be not null";

    notificationsEngineImpl->onSetIndicator(alexaClientSDK::avsCommon::avs::IndicatorState::ON);
    notificationsEngineImpl->shutdown();
}

TEST_F(NotificationsEngineImplTest, verifySetIndicatorForUNKNOWN) {
    EXPECT_CALL(
        *m_alexaMockFactory->getNotificationsMock(), setIndicator(aace::alexa::Notifications::IndicatorState::UNKNOWN));

    auto notificationsEngineImpl = createNotificationsEngineImpl();
    ASSERT_NE(notificationsEngineImpl, nullptr) << "AlertEngineImpl pointer expected to be not null";

    notificationsEngineImpl->onSetIndicator(alexaClientSDK::avsCommon::avs::IndicatorState::UNDEFINED);
    notificationsEngineImpl->shutdown();
}
