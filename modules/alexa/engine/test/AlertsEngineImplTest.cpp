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

#include "AACE/Engine/Alexa/AlertsEngineImpl.h"
#include "AACE/Alexa/Alerts.h"

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/test/MockAVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include "include/MockSpeakerManager.h"
#include <CertifiedSender/CertifiedSender.h>
#include "include/MockAlertsAudioFactoryInterface.h"
#include "include/MockCapabilitiesDelegateInterface.h"
#include "include/MockCustomerDataManager.h"
#include "include/MockMediaPlayer.h"
#include "include/MockMessageObserver.h"
#include "include/MockMessageRouter.h"
#include "include/MockMessageStorage.h"
#include "include/MockSpeaker.h"

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::acl;
using ::testing::Return;

class MockAlertsPlatformInterface : public aace::alexa::Alerts {
public:
    MockAlertsPlatformInterface( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, 
        std::shared_ptr<aace::alexa::Speaker> speaker ) : Alerts(mediaPlayer,speaker) {
    }
    MOCK_METHOD3( alertStateChanged, void(const std::string& alertToken,aace::alexa::Alerts::AlertState state, const std::string& reason) );
    MOCK_METHOD2( alertCreated, void(const std::string& alertToken, const std::string& detailedInfo) );
    MOCK_METHOD1( alertDeleted, void(const std::string& alertToken) );
};

// clang-format off
static const std::string CAPABILITIES_CONFIG_JSON =
    "{"
    "    \"cblAuthDelegate\":{"
    "        \"databaseFilePath\":\"cblAuthDelegate.db\""
    "    },"
    "    \"deviceInfo\":{"
    "        \"deviceSerialNumber\":\"12345678\", "
    "        \"clientId\":\"DummyClientId\","
    "        \"productId\":\"DummyProductID\""
    "    },"
    "    \"capabilitiesDelegate\":{"
    "    },"
    "    \"miscDatabase\":{"
    "        \"databaseFilePath\":\"miscDatabase.db\""
    "    },"
    "    \"alertsCapabilityAgent\":{"
    "        \"databaseFilePath\":\"alerts.db\""
    "    },"
    "    \"settings\":{"
    "        \"databaseFilePath\":\"settings.db\","
    "        \"defaultAVSClientSettings\":{"
    "            \"locale\":\"en-US\""
    "        }"
    "    },"
    "    \"bluetooth\" : {"
    "        \"databaseFilePath\":\"bluetooth.db\""
    "    },"
    "    \"certifiedSender\":{"
    "        \"databaseFilePath\":\"certifiedSender.db\""
    "    },"
    "    \"notifications\":{ "
    "        \"databaseFilePath\":\"notifications.db\""
    "    },"
    "    \"sampleApp\":{"
    "    }"
    " }";

class AlertsEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        }
        auto inString = std::shared_ptr<std::istringstream>(new std::istringstream(CAPABILITIES_CONFIG_JSON));
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize({inString});

        m_mediaPlayer = std::make_shared<aace::test::unit::MockMediaPlayer>();
        m_speaker = std::make_shared<aace::test::unit::MockSpeaker>();

        m_mockAlertsPlatformInterface = std::make_shared<testing::StrictMock<MockAlertsPlatformInterface>> ( 
            m_mediaPlayer, m_speaker);
            
        m_customerDataManager = std::make_shared<testing::StrictMock<MockCustomerDataManager>>();
       
        m_mockMessageRouter = std::make_shared<testing::StrictMock<aace::test::unit::MockMessageRouter>>();
        EXPECT_CALL(*m_mockMessageRouter, setObserver(testing::_)).Times(1);
        EXPECT_CALL(*m_mockMessageRouter, enable()).Times(1);
        m_mockConnectionManager = AVSConnectionManager::create(
            m_mockMessageRouter,
            true,
            std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>(),
            std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::MessageObserverInterface>>());

        m_mockMessageSender = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>();
        m_mockMessageStorage = std::make_shared<MockMessageStorage>();

        EXPECT_CALL(*m_mockMessageStorage, open()).Times(1).WillOnce(Return(true));
        m_certifiedSender = alexaClientSDK::certifiedSender::CertifiedSender::create(
            m_mockMessageSender, m_mockConnectionManager, m_mockMessageStorage, m_customerDataManager);
        
        m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        m_mockFocusManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>>();
        m_mockContextManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockAlertsAudioFactory = std::make_shared<testing::StrictMock<aace::test::unit::MockAlertsAudioFactoryInterface>>();
        m_mockSpeakerManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>>();

        EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockContextManager,setState(testing::_, testing::_,testing::_,testing::_)).Times(testing::Exactly(1));
        m_alertsEngineImpl = aace::engine::alexa::AlertsEngineImpl::create(
            m_mockAlertsPlatformInterface,
            m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
            m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
            m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        } 
    }

    void TearDown() override{
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        } 
        if( m_mockDirectiveSequencer != nullptr ) {
            EXPECT_CALL(*m_mockDirectiveSequencer, doShutdown()).Times(1);
            m_mockDirectiveSequencer->shutdown();
        }
        if( m_mockMessageRouter != nullptr ) {
            EXPECT_CALL(*m_mockMessageRouter, doShutdown()).Times(1);
            m_mockMessageRouter->shutdown();
        }
        if( m_mockConnectionManager != nullptr ) {
            EXPECT_CALL(*m_mockMessageRouter, disable()).Times(1);
            m_mockConnectionManager->shutdown();
        }
        if( m_certifiedSender != nullptr ) {
            m_certifiedSender->shutdown();
        }
        if( m_alertsEngineImpl != nullptr ) {
            m_alertsEngineImpl->shutdown();
        }
        if (avsCommon::avs::initialization::AlexaClientSDKInit::isInitialized()) {
            avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
        }
        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        } 
    }

    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> m_alertsEngineImpl;
    std::shared_ptr<aace::test::unit::MockAlertsPlatformInterface> m_mockAlertsPlatformInterface;
    std::shared_ptr<aace::alexa::MediaPlayer> m_mediaPlayer;
    std::shared_ptr<aace::alexa::Speaker> m_speaker;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_mockConnectionManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender> m_mockMessageSender;
    std::shared_ptr<MockMessageStorage> m_mockMessageStorage;
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> m_certifiedSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>> m_mockFocusManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>> m_mockContextManager;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>> m_mockCapabilitiesDelegate;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockAlertsAudioFactoryInterface>> m_mockAlertsAudioFactory;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>> m_mockSpeakerManager;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCustomerDataManager>> m_customerDataManager;
    std::shared_ptr<MockMessageRouter> m_mockMessageRouter;
};


TEST_F(AlertsEngineImplTest, create) {
    ASSERT_NE(nullptr, m_alertsEngineImpl) << "AlertEngine pointer expected to be not null";
}


TEST_F(AlertsEngineImplTest, reCreateEngineInterface) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;

    EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockContextManager,setState(testing::_, testing::_,testing::_,testing::_)).Times(testing::Exactly(1));
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_NE(nullptr, alertsEngineImplTemp) << "AlertEngine pointer expected to be not null";
    if( alertsEngineImplTemp != nullptr ) {
        alertsEngineImplTemp->shutdown();
    }
}

TEST_F(AlertsEngineImplTest, createWithPlatformInterfaceAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;

    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        nullptr,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}


TEST_F(AlertsEngineImplTest, createWithDirectiveSequencerAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        nullptr, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithMessageSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        nullptr,
        m_mockDirectiveSequencer, nullptr, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithConnectionManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, nullptr, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}


TEST_F(AlertsEngineImplTest, createWithCertifiedSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, nullptr,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithFocusManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        nullptr, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithContextManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, nullptr, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, nullptr,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithExceptionSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        nullptr, m_mockAlertsAudioFactory, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithlertsAudioFactoryAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, nullptr, m_mockSpeakerManager, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertsEngine pointer expected to be null";
}

TEST_F(AlertsEngineImplTest, createWithSpeakerManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;
    
    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, nullptr, m_customerDataManager);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertEngine pointer expected to be  null";
}

TEST_F(AlertsEngineImplTest, createWithCustomerDataManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::AlertsEngineImpl> alertsEngineImplTemp;

    alertsEngineImplTemp = aace::engine::alexa::AlertsEngineImpl::create(
        m_mockAlertsPlatformInterface,
        m_mockDirectiveSequencer, m_mockMessageSender, m_mockConnectionManager, m_certifiedSender,
        m_mockFocusManager, m_mockContextManager, m_mockCapabilitiesDelegate,
        m_mockExceptionSender, m_mockAlertsAudioFactory, m_mockSpeakerManager, nullptr);
    EXPECT_EQ(nullptr, alertsEngineImplTemp) << "AlertEngine pointer expected to be  null";
}

TEST_F(AlertsEngineImplTest, verifyAlertStateChangeForReadyCallback) {
    EXPECT_CALL(*m_mockAlertsPlatformInterface, alertStateChanged(testing::_,aace::alexa::Alerts::AlertState::READY,"READY") );
    m_alertsEngineImpl->onAlertStateChange("DummyToken",
        alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface::State::READY, "READY");
}

TEST_F(AlertsEngineImplTest, verifyAlertStateChangeForErrorCallback) {
    EXPECT_CALL(*m_mockAlertsPlatformInterface, alertStateChanged(testing::_,aace::alexa::Alerts::AlertState::ERROR,"ERROR") );
    m_alertsEngineImpl->onAlertStateChange("DummyToken",
        alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface::State::ERROR, "ERROR");
}

TEST_F(AlertsEngineImplTest, verifyAlertStateChangeForCompletedCallback) {
    EXPECT_CALL(*m_mockAlertsPlatformInterface, alertStateChanged(testing::_,aace::alexa::Alerts::AlertState::COMPLETED,"COMPLETED") );
    m_alertsEngineImpl->onAlertStateChange("DummyToken",
        alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface::State::COMPLETED, "COMPLETED");
}


}  // namespace unit
}  // namespace test
}  // namespace aace