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

#include "AACE/Engine/Alexa/TemplateRuntimeEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>

#include "include/MockAudioPlayerInterface.h"
#include "include/MockCapabilitiesDelegateInterface.h"

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon;

class MockTemplateRuntimePlatformInterface : public aace::alexa::TemplateRuntime {
public:
    MOCK_METHOD1(renderTemplate,
        void(const std::string& payload));
    MOCK_METHOD0(clearTemplate,
        void());
    MOCK_METHOD1(renderPlayerInfo,
        void(const std::string& payload));
    MOCK_METHOD0(clearPlayerInfo,
        void());
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

class TemplateRuntimeEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }
        m_mockTemplateRuntimePlatformInterface = std::make_shared<testing::StrictMock<aace::test::unit::MockTemplateRuntimePlatformInterface>>();
        m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        m_mockAudioPlayerInterface = std::make_shared<testing::StrictMock<aace::test::unit::MockAudioPlayerInterface>>();
        m_mockFocusManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>>();
        m_mockDialogUXStateAggregator = std::make_shared<avsCommon::avs::DialogUXStateAggregator>();
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();

        EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockAudioPlayerInterface, addObserver(testing::NotNull())).Times(testing::Exactly(1));
        
        m_templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
            m_mockTemplateRuntimePlatformInterface, m_mockDirectiveSequencer, m_mockAudioPlayerInterface,
            m_mockFocusManager,m_mockCapabilitiesDelegate, m_mockDialogUXStateAggregator,
            m_mockExceptionSender);

        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    void TearDown() override {
        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }

        if( m_mockDirectiveSequencer != nullptr ) {
            EXPECT_CALL(*m_mockDirectiveSequencer, doShutdown()).Times(1);
            m_mockDirectiveSequencer->shutdown();
        }
        EXPECT_CALL(*m_mockAudioPlayerInterface, removeObserver(testing::NotNull())).Times(testing::Exactly(1));
        if( m_templateRuntimeEngineImpl  != nullptr ) {
            m_templateRuntimeEngineImpl->shutdown();
        }
        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> m_templateRuntimeEngineImpl;
    
    std::shared_ptr<aace::test::unit::MockTemplateRuntimePlatformInterface> m_mockTemplateRuntimePlatformInterface;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;
    std::shared_ptr<aace::test::unit::MockAudioPlayerInterface> m_mockAudioPlayerInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_mockFocusManager;
    std::shared_ptr<testing::StrictMock<aace::test::unit::MockCapabilitiesDelegateInterface>> m_mockCapabilitiesDelegate;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> m_mockDialogUXStateAggregator;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> m_mockExceptionSender;
};

/**
 * Validate SetUp create method
 */
TEST_F( TemplateRuntimeEngineImplTest, create ) {
    EXPECT_NE(nullptr, m_templateRuntimeEngineImpl) << "templateRuntimeEngineImpl pointer is null";
}

/**
 * Test create() with nullptr TemplateRuntimePlatformInterface
 */
TEST_F(TemplateRuntimeEngineImplTest, createWithTemplateRuntimePlatformAsNull) {
    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> templateRuntimeEngineImplTemp;

    templateRuntimeEngineImplTemp = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        nullptr, m_mockDirectiveSequencer, m_mockAudioPlayerInterface,
        m_mockFocusManager, m_mockCapabilitiesDelegate, m_mockDialogUXStateAggregator,
        m_mockExceptionSender);
    EXPECT_EQ(nullptr, templateRuntimeEngineImplTemp) << "templateRuntimeEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr DirectiveSequencer
 */
TEST_F(TemplateRuntimeEngineImplTest, createWithDirectiveSequencerAsNull) {
    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> templateRuntimeEngineImplTemp;

    templateRuntimeEngineImplTemp = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_mockTemplateRuntimePlatformInterface, nullptr, m_mockAudioPlayerInterface,
        m_mockFocusManager, m_mockCapabilitiesDelegate, m_mockDialogUXStateAggregator,
        m_mockExceptionSender);
    EXPECT_EQ(nullptr, templateRuntimeEngineImplTemp) << "templateRuntimeEngineImpl pointer should be null";
}

/**
 * Test create() with nullptr AudioPlayerInterface
 * Should succeed with no call to addObserver for AudioPlayerInterface
 */
TEST_F(TemplateRuntimeEngineImplTest, createWithAudioPlayerAsNull) {
    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> templateRuntimeEngineImplTemp;
    EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));

    templateRuntimeEngineImplTemp = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_mockTemplateRuntimePlatformInterface, m_mockDirectiveSequencer, nullptr,
        m_mockFocusManager, m_mockCapabilitiesDelegate, m_mockDialogUXStateAggregator,
        m_mockExceptionSender);
    EXPECT_NE(nullptr, templateRuntimeEngineImplTemp) << "templateRuntimeEngineImpl pointer should not be null in case of null audioplayer interface";
}

/**
 * Test create() with nullptr FocusManager
 */
TEST_F(TemplateRuntimeEngineImplTest, createWithFocusManagerAsNull) {
    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> templateRuntimeEngineImplTemp;

    templateRuntimeEngineImplTemp = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_mockTemplateRuntimePlatformInterface, m_mockDirectiveSequencer, m_mockAudioPlayerInterface,
        nullptr, m_mockCapabilitiesDelegate, m_mockDialogUXStateAggregator,
        m_mockExceptionSender);
    EXPECT_EQ(nullptr, templateRuntimeEngineImplTemp) << "templateRuntimeEngineImpl pointer should be null";
}
/**
 * Test create() with nullptr CapabilitiesDelegate
 */
TEST_F(TemplateRuntimeEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> templateRuntimeEngineImplTemp;

    templateRuntimeEngineImplTemp = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_mockTemplateRuntimePlatformInterface, m_mockDirectiveSequencer, m_mockAudioPlayerInterface,
        m_mockFocusManager, nullptr, m_mockDialogUXStateAggregator,
        m_mockExceptionSender);
    EXPECT_EQ(nullptr, templateRuntimeEngineImplTemp) << "templateRuntimeEngineImpl pointer should be null";
}
/**
 * Test create() with nullptr DialogUXStateAggregator
 */
TEST_F(TemplateRuntimeEngineImplTest, createWithDialogUXStateAggregatorAsNull) {
    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> templateRuntimeEngineImplTemp;

    templateRuntimeEngineImplTemp = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_mockTemplateRuntimePlatformInterface, m_mockDirectiveSequencer, m_mockAudioPlayerInterface,
        m_mockFocusManager, m_mockCapabilitiesDelegate, nullptr,
        m_mockExceptionSender);
    EXPECT_EQ(nullptr, templateRuntimeEngineImplTemp) << "templateRuntimeEngineImpl pointer should be null";
}
/**
 * Test create() with nullptr ExceptionSender
 */
TEST_F(TemplateRuntimeEngineImplTest, createWithExceptionSenderAsNull) {
    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> templateRuntimeEngineImplTemp;

    templateRuntimeEngineImplTemp = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_mockTemplateRuntimePlatformInterface, m_mockDirectiveSequencer, m_mockAudioPlayerInterface,
        m_mockFocusManager, m_mockCapabilitiesDelegate, m_mockDialogUXStateAggregator,
        nullptr);
    EXPECT_EQ(nullptr, templateRuntimeEngineImplTemp) << "templateRuntimeEngineImpl pointer should be null";
}

/**
 * Test render template
 */
TEST_F( TemplateRuntimeEngineImplTest, testRenderTemplate ) {
    const std::string& testJsonPayload = CAPABILITIES_CONFIG_JSON;
    EXPECT_CALL(*m_mockTemplateRuntimePlatformInterface, renderTemplate(testing::StrEq(testJsonPayload)));
    m_templateRuntimeEngineImpl->renderTemplateCard(testJsonPayload, alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND);
}

/**
 * Test clear template
 */
TEST_F( TemplateRuntimeEngineImplTest, testClearTemplateCard ) {
    EXPECT_CALL(*m_mockTemplateRuntimePlatformInterface, clearTemplate());
    m_templateRuntimeEngineImpl->clearTemplateCard();
}

/**
 * Test player info
 */
TEST_F( TemplateRuntimeEngineImplTest, testRenderPlayerInfoCard ) {
    const std::string& testJsonPayload = CAPABILITIES_CONFIG_JSON;
    alexaClientSDK::avsCommon::sdkInterfaces::TemplateRuntimeObserverInterface::AudioPlayerInfo audioPlayerInfo;
    audioPlayerInfo = alexaClientSDK::avsCommon::sdkInterfaces::TemplateRuntimeObserverInterface::AudioPlayerInfo();
    EXPECT_CALL(*m_mockTemplateRuntimePlatformInterface, renderPlayerInfo(testing::StrEq(testJsonPayload)));
    m_templateRuntimeEngineImpl->renderPlayerInfoCard(testJsonPayload, audioPlayerInfo, alexaClientSDK::avsCommon::avs::FocusState::BACKGROUND);
}

/**
 * Test clear player info
 */
TEST_F( TemplateRuntimeEngineImplTest, testclearPlayerInfoCard ) {
    EXPECT_CALL(*m_mockTemplateRuntimePlatformInterface, clearPlayerInfo());
    m_templateRuntimeEngineImpl->clearPlayerInfoCard();
}

}  // namespace unit
}  // namespace test
}  // namespace aace

