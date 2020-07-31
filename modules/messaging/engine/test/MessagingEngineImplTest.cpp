/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include "AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h"
#include "AVSCommon/SDKInterfaces/test/MockContextManager.h"

#include "AACE/Test/Alexa/AlexaTestHelper.h"
#include "AACE/Messaging/Messaging.h"
#include "AACE/Engine/Messaging/MessagingEngineImpl.h"

namespace aace {
namespace test {
namespace unit {

/**
 * Mock Messaging platform interface.
 */
class MockMessagingPlatformInterface : public aace::messaging::Messaging {
public:
    MOCK_METHOD3(
        sendMessage,
        void(const std::string& token, const std::string& message, const std::string& recipients));
    MOCK_METHOD1(uploadConversations, void(const std::string& token));
    MOCK_METHOD3(
        updateMessagesStatus,
        void(const std::string& token, const std::string& conversationId, const std::string& status));
};

/**
 * Unit test creation of MessagingEngineImpl class.
 */
class MessagingEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockPlatformInterface = std::make_shared<testing::StrictMock<MockMessagingPlatformInterface>>();
        m_mockExceptionEncounteredSender = std::make_shared<
            testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockContextManager =
            std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_alexaMockFactory = alexa::AlexaTestHelper::createAlexaMockComponentFactory();
        m_messagingEngineImpl = aace::engine::messaging::MessagingEngineImpl::create(
            m_mockPlatformInterface,
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock());
    }

    void TearDown() override {
        m_messagingEngineImpl->shutdown();
        m_alexaMockFactory->shutdown();
    }

    std::shared_ptr<aace::engine::messaging::MessagingEngineImpl> m_messagingEngineImpl;

    /// @c Messaging platform interface mock implementation
    std::shared_ptr<aace::messaging::Messaging> m_mockPlatformInterface;

    /// Mock exception sender reference
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>
        m_mockExceptionEncounteredSender;

    /// Mock context manager reference
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>
        m_mockContextManager;

    // Mock alexa component factory reference
    std::shared_ptr<alexa::AlexaMockComponentFactory> m_alexaMockFactory;
};

/**
 * Test create() with nullptrs
 */
TEST_F(MessagingEngineImplTest, create) {
    EXPECT_NE(nullptr, m_messagingEngineImpl);
}

TEST_F(MessagingEngineImplTest, createWithNullPlatform) {
    std::shared_ptr<aace::engine::messaging::MessagingEngineImpl> testMessagingEngineImpl;
    testMessagingEngineImpl = engine::messaging::MessagingEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_mockExceptionEncounteredSender,
        m_mockContextManager,
        m_alexaMockFactory->getMessageSenderInterfaceMock());
    EXPECT_EQ(nullptr, testMessagingEngineImpl);
}

TEST_F(MessagingEngineImplTest, createWithNullEndpointBuilder) {
    std::shared_ptr<aace::engine::messaging::MessagingEngineImpl> testMessagingEngineImpl;
    testMessagingEngineImpl = engine::messaging::MessagingEngineImpl::create(
        m_mockPlatformInterface,
        nullptr,
        m_mockExceptionEncounteredSender,
        m_mockContextManager,
        m_alexaMockFactory->getMessageSenderInterfaceMock());
    EXPECT_EQ(nullptr, testMessagingEngineImpl);
}

TEST_F(MessagingEngineImplTest, createWithNullExceptionEncounteredSender) {
    std::shared_ptr<aace::engine::messaging::MessagingEngineImpl> testMessagingEngineImpl;
    testMessagingEngineImpl = engine::messaging::MessagingEngineImpl::create(
        m_mockPlatformInterface,
        m_alexaMockFactory->getEndpointBuilderMock(),
        nullptr,
        m_mockContextManager,
        m_alexaMockFactory->getMessageSenderInterfaceMock());
    EXPECT_EQ(nullptr, testMessagingEngineImpl);
}

TEST_F(MessagingEngineImplTest, createWithNullContextManager) {
    std::shared_ptr<aace::engine::messaging::MessagingEngineImpl> testMessagingEngineImpl;
    testMessagingEngineImpl = engine::messaging::MessagingEngineImpl::create(
        m_mockPlatformInterface,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_mockExceptionEncounteredSender,
        nullptr,
        m_alexaMockFactory->getMessageSenderInterfaceMock());
    EXPECT_EQ(nullptr, testMessagingEngineImpl);
}

TEST_F(MessagingEngineImplTest, createWithNullMessageSender) {
    std::shared_ptr<aace::engine::messaging::MessagingEngineImpl> testMessagingEngineImpl;
    testMessagingEngineImpl = engine::messaging::MessagingEngineImpl::create(
        m_mockPlatformInterface,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_mockExceptionEncounteredSender,
        m_mockContextManager,
        nullptr);
    EXPECT_EQ(nullptr, testMessagingEngineImpl);
}

}  // namespace unit
}  // namespace test
}  // namespace aace
