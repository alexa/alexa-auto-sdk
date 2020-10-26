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
#include <AACE/Test/Alexa/MockTemplateRuntime.h>

#include <AACE/Engine/Alexa/TemplateRuntimeEngineImpl.h>

using namespace aace::test::alexa;
using namespace aace::test::audio;

class TemplateRuntimeEngineImplTest : public ::testing::Test {
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
            EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

            m_configured = true;
        }
    }

    std::shared_ptr<aace::engine::alexa::TemplateRuntimeEngineImpl> createTemplateRuntimeEngineImpl() {
        if (m_configured == false) {
            configure();
        }

        auto templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
            m_alexaMockFactory->getTemplateRuntimeMock(),
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_playerInfoInterfaceMock,
            m_alexaMockFactory->getFocusManagerInterfaceMock(),
            m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
            m_alexaMockFactory->getDialogUXStateAggregatorMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

        return templateRuntimeEngineImpl;
    }

    std::unordered_set<
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>>
        m_playerInfoInterfaceMock;

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

TEST_F(TemplateRuntimeEngineImplTest, create) {
    auto templateRuntimeEngineImpl = createTemplateRuntimeEngineImpl();
    ASSERT_NE(templateRuntimeEngineImpl, nullptr) << "TemplateRuntimeEngineImpl pointer expected to be not null";

    templateRuntimeEngineImpl->shutdown();
}

TEST_F(TemplateRuntimeEngineImplTest, createWithPlatformInterfaceAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_playerInfoInterfaceMock,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(templateRuntimeEngineImpl, nullptr) << "TemplateRuntimeEngineImpl pointer expected to be null";
}

TEST_F(TemplateRuntimeEngineImplTest, createWithDirectiveSequencerAsNull) {
    auto templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_alexaMockFactory->getTemplateRuntimeMock(),
        nullptr,
        m_playerInfoInterfaceMock,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(templateRuntimeEngineImpl, nullptr) << "TemplateRuntimeEngineImpl pointer expected to be null";
}

TEST_F(TemplateRuntimeEngineImplTest, createWithFocusManagerAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_alexaMockFactory->getTemplateRuntimeMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_playerInfoInterfaceMock,
        nullptr,
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(templateRuntimeEngineImpl, nullptr) << "TemplateRuntimeEngineImpl pointer expected to be null";
}

TEST_F(TemplateRuntimeEngineImplTest, createWithCapabilitiesDelegateAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_alexaMockFactory->getTemplateRuntimeMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_playerInfoInterfaceMock,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(templateRuntimeEngineImpl, nullptr) << "TemplateRuntimeEngineImpl pointer expected to be null";
}

TEST_F(TemplateRuntimeEngineImplTest, createWithDialogUXStateAggregatorAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_alexaMockFactory->getTemplateRuntimeMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_playerInfoInterfaceMock,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        nullptr,
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock());

    ASSERT_EQ(templateRuntimeEngineImpl, nullptr) << "TemplateRuntimeEngineImpl pointer expected to be null";
}

TEST_F(TemplateRuntimeEngineImplTest, createWithExceptionSenderAsNull) {
    EXPECT_CALL(*m_alexaMockFactory->getDirectiveSequencerInterfaceMock(), doShutdown());

    auto templateRuntimeEngineImpl = aace::engine::alexa::TemplateRuntimeEngineImpl::create(
        m_alexaMockFactory->getTemplateRuntimeMock(),
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_playerInfoInterfaceMock,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getCapabilitiesDelegateInterfaceMock(),
        m_alexaMockFactory->getDialogUXStateAggregatorMock(),
        nullptr);

    ASSERT_EQ(templateRuntimeEngineImpl, nullptr) << "TemplateRuntimeEngineImpl pointer expected to be null";
}

TEST_F(TemplateRuntimeEngineImplTest, validateTemplateRuntimeCallbacks) {
    auto templateRuntimeEngineImpl = createTemplateRuntimeEngineImpl();
    ASSERT_NE(templateRuntimeEngineImpl, nullptr) << "TemplateRuntimeEngineImpl pointer expected to be not null";

    EXPECT_CALL(
        *m_alexaMockFactory->getTemplateRuntimeMock(),
        renderTemplate(testing::StrEq("TEMPLATE_PAYLOAD"), testing::Eq(aace::alexa::FocusState::FOREGROUND)));
    templateRuntimeEngineImpl->renderTemplateCard(
        "TEMPLATE_PAYLOAD", alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND);

    EXPECT_CALL(*m_alexaMockFactory->getTemplateRuntimeMock(), clearTemplate());
    templateRuntimeEngineImpl->clearTemplateCard();

    EXPECT_CALL(
        *m_alexaMockFactory->getTemplateRuntimeMock(),
        renderPlayerInfo(
            testing::StrEq("PLAYER_INFO_PAYLOAD"),
            testing::Eq(aace::alexa::PlayerActivity::IDLE),
            testing::Eq(std::chrono::milliseconds::zero()),
            testing::Eq(aace::alexa::FocusState::FOREGROUND)));
    templateRuntimeEngineImpl->renderPlayerInfoCard(
        "PLAYER_INFO_PAYLOAD",
        alexaClientSDK::avsCommon::sdkInterfaces::TemplateRuntimeObserverInterface::AudioPlayerInfo(),
        alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND);

    EXPECT_CALL(*m_alexaMockFactory->getTemplateRuntimeMock(), clearPlayerInfo());
    templateRuntimeEngineImpl->clearPlayerInfoCard();

    templateRuntimeEngineImpl->shutdown();
}
