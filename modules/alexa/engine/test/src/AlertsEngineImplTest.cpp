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
#include <AACE/Test/Alexa/MockAlerts.h>

#include <AACE/Engine/Alexa/AlertsEngineImpl.h>
#include <AACE/Alexa/Alerts.h>

using namespace aace::test::alexa;
using namespace aace::test::audio;

class AlertsEngineImplTest : public ::testing::Test {
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
            EXPECT_CALL(*m_alexaMockFactory->getAudioManagerMock(),openAudioOutputChannel("Alerts",aace::audio::AudioOutputProvider::AudioOutputType::ALARM))
                .WillOnce(testing::Return(m_alexaMockFactory->getAudioOutputChannelMock()));
            EXPECT_CALL(*m_alexaMockFactory->getAudioOutputChannelMock(),setEngineInterface(testing::_)).Times(testing::Exactly(2));;
            EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
            EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
            EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());
            EXPECT_CALL(*m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),registerCapability(testing::_)).WillOnce(testing::Return(true));
            EXPECT_CALL(*m_alexaMockFactory->getContextManagerInterfaceMock(),setState(testing::_, testing::_,testing::_,testing::_));
            EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
            EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
            EXPECT_CALL(*m_alexaMockFactory->getFocusManagerInterfaceMock(),addObserver(testing::_));
            EXPECT_CALL(*m_alexaMockFactory->getSpeakerManagerInterfaceMock(), addSpeakerManagerObserver(testing::_)).Times(testing::AtLeast(1));
            EXPECT_CALL(*m_alexaMockFactory->getSpeakerManagerInterfaceMock(), addSpeaker(testing::_)).Times(testing::AtLeast(1));
            EXPECT_CALL(*m_alexaMockFactory->getSpeakerManagerInterfaceMock(), getSpeakerSettings(testing::_,testing::_))
                .WillRepeatedly(testing::Invoke([](alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type, alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings*) {
                std::promise<bool> promise;
                promise.set_value(true);
                return promise.get_future();
            }));
            
            m_configured = true;
        }
    }
    
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> createAlertsEngineImpl()
    {
        if( m_configured == false ) {
            configure();
        }
        
        auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
            m_alexaMockFactory->getAlertsMock(),
            m_alexaMockFactory->getAudioManagerMock(),
            m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            m_alexaMockFactory->getAVSConnectionManagerMock(),
            m_alexaMockFactory->getCertifiedSenderMock(),
            m_alexaMockFactory->getFocusManagerInterfaceMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock(),
            m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
            m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
            m_alexaMockFactory->getCustomerDataManagerMock() );
        
        return alertsEngineImpl;
    }
    
protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

TEST_F(AlertsEngineImplTest, create)
{
    auto alertsEngineImpl = createAlertsEngineImpl();
    ASSERT_NE(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be not null";
    
    alertsEngineImpl->shutdown();
}

TEST_F(AlertsEngineImplTest,createWithPlatformInterfaceAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithAudioManagerInterfaceAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        nullptr,
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithDirectiveSequencerAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        nullptr,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithMessageSenderAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithConnectionManagerAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithCertifiedSenderAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        nullptr,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithFocusManagerAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        nullptr,
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithContextManagerAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithCapabilitiesDelegateAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithExceptionSenderAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithlAudioFactoryAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithSpeakerManagerAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getCustomerDataManagerMock() );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,createWithCustomerDataManagerAsNull)
{
    EXPECT_CALL(*m_alexaMockFactory->getMessageStorageMock(),open()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),addConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getAVSConnectionManagerMock(),removeConnectionStatusObserver(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),doShutdown());

    auto alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
        m_alexaMockFactory->getAlertsMock(),
        m_alexaMockFactory->getAudioManagerMock(),
        m_alexaMockFactory->getDirectiveSequencerInterfaceMock(),
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_alexaMockFactory->getAVSConnectionManagerMock(),
        m_alexaMockFactory->getCertifiedSenderMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getContextManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
        m_alexaMockFactory->getAlertsAudioFactoryInterfaceMock(),
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),
        nullptr );
    
    ASSERT_EQ(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be null";
}

TEST_F(AlertsEngineImplTest,verifyAlertStateCallbacks)
{
    auto alertsEngineImpl = createAlertsEngineImpl();
    ASSERT_NE(alertsEngineImpl,nullptr) << "AlertEngineImpl pointer expected to be not null";
    
    EXPECT_CALL(*m_alexaMockFactory->getAlertsMock(),alertStateChanged("TOKEN",aace::alexa::Alerts::AlertState::READY,"REASON_UNIT_TEST"));
    alertsEngineImpl->onAlertStateChange( "TOKEN", "TYPE", alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface::State::READY, "REASON_UNIT_TEST" );

    EXPECT_CALL(*m_alexaMockFactory->getAlertsMock(),alertStateChanged("TOKEN",aace::alexa::Alerts::AlertState::ERROR, "REASON_UNIT_TEST"));
    alertsEngineImpl->onAlertStateChange( "TOKEN", "TYPE", alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface::State::ERROR, "REASON_UNIT_TEST");

    EXPECT_CALL(*m_alexaMockFactory->getAlertsMock(),alertStateChanged("TOKEN",aace::alexa::Alerts::AlertState::COMPLETED, "REASON_UNIT_TEST"));
    alertsEngineImpl->onAlertStateChange( "TOKEN", "TYPE", alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface::State::COMPLETED, "REASON_UNIT_TEST");

    alertsEngineImpl->shutdown();
}

