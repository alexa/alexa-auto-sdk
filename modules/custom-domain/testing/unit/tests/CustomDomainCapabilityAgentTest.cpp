/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <future>

#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveHandlerResult.h>
#include <AVSCommon/AVS/Attachment/AttachmentManagerInterface.h>
#include <AACE/Test/Unit/AVS/MockAttachmentManager.h>
#include <AVSCommon/Utils/WaitEvent.h>
#include <AACE/Engine/CustomDomain/CustomDomainCapabilityAgent.h>

namespace aace {
namespace test {
namespace unit {

using namespace alexaClientSDK::avsCommon::avs::attachment;
using namespace alexaClientSDK::avsCommon::utils::sds;
using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;

// clang-format off
static const std::string TEST_CONTEXT = R"(
    {
        "context": [
        {
            "name": "TEST_STATE_1",
            "value": "test"
        },
        {
            "name": "TEST_STATE_2",
            "value": "test"
        }
        ]
    }
)";
static const std::string TEST_EVENT_PAYLOAD = R"(
    {
        "testing_key": "testing_value"
    }
)";
// clang-format on

static std::chrono::milliseconds TIMEOUT(1000);

class MockHandler : public aace::engine::customDomain::CustomDomainHandlerInterface {
public:
    MOCK_METHOD5(
        handleDirective,
        void(
            const std::string& directiveNamespace,
            const std::string& name,
            const std::string& payload,
            const std::string& correlationToken,
            const std::string& messageId));
    MOCK_METHOD4(
        cancelDirective,
        void(
            const std::string& directiveNamespace,
            const std::string& name,
            const std::string& correlationToken,
            const std::string& messageId));
    MOCK_METHOD1(getContext, std::string(const std::string& contextNamespace));
};

class CustomDomainCapabilityAgentTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockContextManager =
            std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockExceptionSender = std::make_shared<
            testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockMessageSender =
            std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>>();
        m_mockDirectiveHandlerResult = std::unique_ptr<
            testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveHandlerResult>>();
        m_mockHandler = std::make_shared<testing::StrictMock<MockHandler>>();
        m_capAgent = aace::engine::customDomain::CustomDomainCapabilityAgent::create(
            "TEST_NAMESPACE",
            "TEST_VERSION",
            {},
            m_mockHandler,
            m_mockExceptionSender,
            m_mockContextManager,
            m_mockMessageSender);
    }

    void TearDown() override {
        m_capAgent->shutdown();
    }

    CustomDomainCapabilityAgentTest() :
            m_wakeSetCompletedPromise{}, m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()} {
    }

    std::promise<void> m_wakeSetCompletedPromise;
    std::future<void> m_wakeSetCompletedFuture;

    std::shared_ptr<aace::engine::customDomain::CustomDomainCapabilityAgent> m_capAgent;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>
        m_mockContextManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>
        m_mockExceptionSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>>
        m_mockMessageSender;
    std::unique_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveHandlerResult>>
        m_mockDirectiveHandlerResult;
    std::shared_ptr<testing::StrictMock<MockHandler>> m_mockHandler;
};

TEST_F(CustomDomainCapabilityAgentTest, create) {
    EXPECT_NE(nullptr, m_capAgent);
}

TEST_F(CustomDomainCapabilityAgentTest, createWithNullContextManager) {
    std::shared_ptr<aace::engine::customDomain::CustomDomainCapabilityAgent> capAgent;
    capAgent = aace::engine::customDomain::CustomDomainCapabilityAgent::create(
        "TEST_NAMESPACE", "TEST_VERSION", {}, m_mockHandler, m_mockExceptionSender, nullptr, m_mockMessageSender);
    EXPECT_EQ(nullptr, capAgent);
}

TEST_F(CustomDomainCapabilityAgentTest, createWithNullExceptionSender) {
    std::shared_ptr<aace::engine::customDomain::CustomDomainCapabilityAgent> capAgent;
    capAgent = aace::engine::customDomain::CustomDomainCapabilityAgent::create(
        "TEST_NAMESPACE", "TEST_VERSION", {}, m_mockHandler, nullptr, m_mockContextManager, m_mockMessageSender);
    EXPECT_EQ(nullptr, capAgent);
}

TEST_F(CustomDomainCapabilityAgentTest, createWithNullMessageSender) {
    std::shared_ptr<aace::engine::customDomain::CustomDomainCapabilityAgent> capAgent;
    capAgent = aace::engine::customDomain::CustomDomainCapabilityAgent::create(
        "TEST_NAMESPACE", "TEST_VERSION", {}, m_mockHandler, m_mockExceptionSender, m_mockContextManager, nullptr);
    EXPECT_EQ(nullptr, capAgent);
}

TEST_F(CustomDomainCapabilityAgentTest, testArbitraryCustomDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(
        "TEST_NAMESPACE", "TEST_NAME", "TEST_MESSAGE_ID", "", "TEST_CORRELATION_TOKEN");
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, "TEST_PAYLOAD", attachmentManager, "");

    EXPECT_CALL(
        *m_mockHandler,
        handleDirective("TEST_NAMESPACE", "TEST_NAME", "TEST_PAYLOAD", "TEST_CORRELATION_TOKEN", "TEST_MESSAGE_ID"))
        .Times(testing::Exactly(1));

    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective("TEST_MESSAGE_ID");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(CustomDomainCapabilityAgentTest, testSendEventWithoutContextRequired) {
    EXPECT_CALL(*m_mockContextManager, getContext(testing::_, testing::_, testing::_)).Times(testing::Exactly(0));
    EXPECT_CALL(*m_mockHandler, getContext(testing::_)).Times(testing::Exactly(0));
    EXPECT_CALL(*m_mockMessageSender, sendMessage(testing::_)).Times(testing::Exactly(1));
    m_capAgent->sendEvent("TEST_NAME", TEST_EVENT_PAYLOAD, false, "", "");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(CustomDomainCapabilityAgentTest, testSendEventWithContextRequiredWithoutCachedContext) {
    EXPECT_CALL(*m_mockContextManager, getContext(testing::_, testing::_, testing::_)).Times(testing::Exactly(1));
    m_capAgent->sendEvent("TEST_NAME", TEST_EVENT_PAYLOAD, true, "", "");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(CustomDomainCapabilityAgentTest, testSendEventWithContextRequiredWithCachedContextInWrongFormat) {
    EXPECT_CALL(*m_mockContextManager, addStateProvider(testing::_, ::testing::NotNull())).Times(testing::Exactly(2));
    ;
    std::shared_ptr<aace::engine::customDomain::CustomDomainCapabilityAgent> capAgent;
    std::vector<std::string> states{"TEST_STATE_1", "TEST_STATE_2"};
    capAgent = aace::engine::customDomain::CustomDomainCapabilityAgent::create(
        "TEST_NAMESPACE",
        "TEST_VERSION",
        states,
        m_mockHandler,
        m_mockExceptionSender,
        m_mockContextManager,
        m_mockMessageSender);

    EXPECT_CALL(*m_mockContextManager, getContext(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1))
        .WillOnce(testing::Return(42));
    EXPECT_CALL(*m_mockContextManager, provideStateResponse(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(2))
        .WillOnce(testing::Return())
        .WillOnce(testing::Return());
    EXPECT_CALL(*m_mockHandler, getContext(testing::_))
        .Times(testing::Exactly(1))
        .WillOnce(testing::Return(TEST_CONTEXT));
    EXPECT_CALL(*m_mockMessageSender, sendMessage(testing::_)).Times(testing::Exactly(1));
    capAgent->sendEvent("TEST_NAME", TEST_EVENT_PAYLOAD, true, "", "MALFORMED_CONTEXT");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
    for (const auto& state : states) {
        capAgent->provideState(NamespaceAndName{"TEST_NAMESPACE", state}, 42);
    }
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
    capAgent->onContextAvailable("", AVSContext(), 42);
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(CustomDomainCapabilityAgentTest, testSendEventWithContextRequiredWithCachedContextInCorrectFormat) {
    EXPECT_CALL(*m_mockContextManager, addStateProvider(testing::_, ::testing::NotNull())).Times(testing::Exactly(2));
    ;
    std::shared_ptr<aace::engine::customDomain::CustomDomainCapabilityAgent> capAgent;
    std::vector<std::string> states{"TEST_STATE_1", "TEST_STATE_2"};
    capAgent = aace::engine::customDomain::CustomDomainCapabilityAgent::create(
        "TEST_NAMESPACE",
        "TEST_VERSION",
        states,
        m_mockHandler,
        m_mockExceptionSender,
        m_mockContextManager,
        m_mockMessageSender);

    EXPECT_CALL(*m_mockContextManager, getContext(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1))
        .WillOnce(testing::Return(42));
    EXPECT_CALL(*m_mockContextManager, provideStateResponse(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(2))
        .WillOnce(testing::Return())
        .WillOnce(testing::Return());
    EXPECT_CALL(*m_mockHandler, getContext(testing::_)).Times(testing::Exactly(0));
    EXPECT_CALL(*m_mockMessageSender, sendMessage(testing::_)).Times(testing::Exactly(1));
    capAgent->sendEvent("TEST_NAME", TEST_EVENT_PAYLOAD, true, "", TEST_CONTEXT);
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
    for (const auto& state : states) {
        capAgent->provideState(NamespaceAndName{"TEST_NAMESPACE", state}, 42);
    }
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
    capAgent->onContextAvailable("", AVSContext(), 42);
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(CustomDomainCapabilityAgentTest, testSendEventWithInvalidPayload) {
    EXPECT_CALL(*m_mockContextManager, getContext(testing::_, testing::_, testing::_)).Times(testing::Exactly(0));
    EXPECT_CALL(*m_mockHandler, getContext(testing::_)).Times(testing::Exactly(0));
    EXPECT_CALL(*m_mockMessageSender, sendMessage(testing::_)).Times(testing::Exactly(0));
    m_capAgent->sendEvent("TEST_NAME", "", false, "", "");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(CustomDomainCapabilityAgentTest, testProvideSingleState) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;

    EXPECT_CALL(*m_mockContextManager, addStateProvider(testing::_, ::testing::NotNull()));
    std::shared_ptr<aace::engine::customDomain::CustomDomainCapabilityAgent> capAgent;
    capAgent = aace::engine::customDomain::CustomDomainCapabilityAgent::create(
        "TEST_NAMESPACE",
        "TEST_VERSION",
        {"TEST_STATE_1"},
        m_mockHandler,
        m_mockExceptionSender,
        m_mockContextManager,
        m_mockMessageSender);
    EXPECT_NE(nullptr, capAgent);
    EXPECT_CALL(*m_mockContextManager, provideStateUnavailableResponse(testing::_, testing::_, testing::_))
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() { waitEvent.wakeUp(); }));
    EXPECT_CALL(*m_mockHandler, getContext(testing::_)).Times(testing::Exactly(1));

    capAgent->provideState(NamespaceAndName{"TEST_NAMESPACE", "TEST_STATE_1"}, 42);
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(CustomDomainCapabilityAgentTest, testProvideMultipleState) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;

    EXPECT_CALL(*m_mockContextManager, addStateProvider(testing::_, ::testing::NotNull())).Times(testing::Exactly(2));
    std::shared_ptr<aace::engine::customDomain::CustomDomainCapabilityAgent> capAgent;
    std::vector<std::string> states{"TEST_STATE_1", "TEST_STATE_2"};
    capAgent = aace::engine::customDomain::CustomDomainCapabilityAgent::create(
        "TEST_NAMESPACE",
        "TEST_VERSION",
        states,
        m_mockHandler,
        m_mockExceptionSender,
        m_mockContextManager,
        m_mockMessageSender);
    EXPECT_NE(nullptr, capAgent);
    EXPECT_CALL(*m_mockContextManager, provideStateResponse(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(2))
        .WillOnce(testing::Return())
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() { waitEvent.wakeUp(); }));
    EXPECT_CALL(*m_mockHandler, getContext(testing::_))
        .Times(testing::Exactly(1))
        .WillOnce(testing::Return(TEST_CONTEXT));

    for (const auto& state : states) {
        capAgent->provideState(NamespaceAndName{"TEST_NAMESPACE", state}, 42);
    }
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(CustomDomainCapabilityAgentTest, testCancelDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(
        "TEST_NAMESPACE", "TEST_NAME", "TEST_MESSAGE_ID", "", "TEST_CORRELATION_TOKEN");
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, "TEST_PAYLOAD", attachmentManager, "");

    EXPECT_CALL(*m_mockHandler, cancelDirective(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::cancelDirective("TEST_MESSAGE_ID");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

}  // namespace unit
}  // namespace test
}  // namespace aace
