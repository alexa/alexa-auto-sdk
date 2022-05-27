/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <nlohmann/json.hpp>
#include <regex>

#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveHandlerResult.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>
#include <AVSCommon/AVS/EventBuilder.h>

#include <AACE/Test/Unit/AVS/MockAttachmentManager.h>

#include <AACE/Engine/PhoneCallController/PhoneCallControllerCapabilityAgent.h>

namespace aace {
namespace test {
namespace unit {
namespace phoneCallController {

static const std::string TEST_CALL_ID = "1234";

static const std::string DIAL_PAYLOAD =
    "{"
    "\"callId\":\"CALLID\","
    "\"callee\":{"
    "\"description\":\"DESCRIPTION\","
    "\"defaultContactAddresses\": {"
    "\"protocol\": \"PROTOCOL\","
    "\"format\": \"FORMAT\","
    "\"value\": \"VALUE\""
    "},"
    "\"alternativeContactAddresses\": [{"
    "\"protocol\": \"PROTOCOL\","
    "\"format\": \"FORMAT\","
    "\"value\": \"VALUE\""
    "}]"
    "},"
    "\"mediaOptions\":{"
    "\"audiOnly\":\"BOOLEAN\""
    "}"
    "}";

static const std::string DIAL_PAYLOAD_NO_CALLID =
    "{"
    "\"callee\":{"
    "\"description\":\"DESCRIPTION\","
    "\"defaultContactAddresses\": {"
    "\"protocol\": \"PROTOCOL\","
    "\"format\": \"FORMAT\","
    "\"value\": \"VALUE\""
    "},"
    "\"alternativeContactAddresses\": [{"
    "\"protocol\": \"PROTOCOL\","
    "\"format\": \"FORMAT\","
    "\"value\": \"VALUE\""
    "}]"
    "},"
    "\"mediaOptions\":{"
    "\"audiOnly\":\"BOOLEAN\""
    "}"
    "}";

static const std::string DIAL_PAYLOAD_BAD_FORMAT =
    "{"
    "\"callId\":\"CALLID\""
    "\"callee\":{,"
    "\"description\":\"DESCRIPTION\","
    "\"defaultContactAddresses\": {"
    "\"protocol\": \"PROTOCOL\","
    "\"format\": \"FORMAT\","
    "\"value\": \"VALUE\""
    "},"
    "\"alternativeContactAddresses\": [{"
    "\"protocol\": \"PROTOCOL\","
    "\"format\": \"FORMAT\","
    "\"value\": \"VALUE\""
    "}]"
    "},"
    "\"mediaOptions\":{"
    "\"audiOnly\":\"BOOLEAN\""
    "}"
    "}";

static const std::string REDIAL_PAYLOAD =
    "{"
    "\"callId\":\"CALLID\""
    "}";

static const std::string STOP_PAYLOAD =
    "{"
    "\"callId\":\"CALLID\""
    "}";

static const std::string ANSWER_PAYLOAD =
    "{"
    "\"callId\":\"CALLID\","
    "\"mediaOptions\":{"
    "\"audiOnly\":\"BOOLEAN\""
    "}"
    "}";

static const std::string SENDDTMF_PAYLOAD =
    "{"
    "\"callId\":\"CALLID\","
    "\"signal\":\"SIGNAL\""
    "}";

static const std::string PHONE_CONNECTED_CONTEXT =
    R"({
        "device": {
            "connectionState": "CONNECTED"
        },
        "configuration": {
            "callingFeature": [
                {
                    "OVERRIDE_RINGTONE_SUPPORTED": false
                }
            ]
        },
        "allCalls": []
    })";

static const std::string CALL_FAILED_EVENT_NAME = "CallFailed";

static const std::string CALL_FAILED_NO_ANSWER_EVENT_PAYLOAD =
    R"({
        "callId": "1234",
        "error": {
            "code": "NO_ANSWER",
            "message": "TEST"
        }
    })";

static const std::string CALL_FAILED_CONTEXT =
    R"({
        "device": {
            "connectionState": "CONNECTED"
        },
        "configuration": {
            "callingFeature": [
                {
                    "OVERRIDE_RINGTONE_SUPPORTED": false
                }
            ]
        },
        "allCalls": []
    })";

static const std::string DIAL_STARTED_EVENT_NAME = "DialStarted";

static const std::string DIAL_STARTED_EVENT_PAYLOAD =
    R"({
        "callId": "1234"
    })";

static const std::string DIAL_STARTED_CONTEXT =
    R"({
        "device": {
            "connectionState": "CONNECTED"
        },
        "configuration": {
            "callingFeature": [
                {
                    "OVERRIDE_RINGTONE_SUPPORTED": false
                }
            ]
        },
        "allCalls": [
            {
                "callId": "1234",
                "callState": "TRYING"
            }
        ],
        "currentCall": {
            "callId": "1234"
        }
    })";

static const std::string CALL_TERMINATED_EVENT_NAME = "CallTerminated";

static const std::string CALL_TERMINATED_EVENT_PAYLOAD =
    R"({
        "callId": "1234"
    })";

static const std::string CALL_TERMINATED_CONTEXT =
    R"({
        "device": {
            "connectionState": "CONNECTED"
        },
        "configuration": {
            "callingFeature": [
                {
                    "OVERRIDE_RINGTONE_SUPPORTED": false
                }
            ]
        },
        "allCalls": []
    })";

static const std::string CALL_ACTIVATED_EVENT_NAME = "CallActivated";

static const std::string CALL_ACTIVATED_EVENT_PAYLOAD =
    R"({
        "callId": "1234"
    })";

static const std::string CALL_ACTIVATED_CONTEXT =
    R"({
        "device": {
            "connectionState": "CONNECTED"
        },
        "configuration": {
            "callingFeature": [
                {
                    "OVERRIDE_RINGTONE_SUPPORTED": false
                }
            ]
        },
        "allCalls": [
            {
                "callId": "1234",
                "callState": "ACTIVE"
            }
        ],
        "currentCall": {
            "callId": "1234"
        }
    })";

static const std::string OUTBOUND_RINGING_STARTED_EVENT_NAME = "OutboundRingingStarted";

static const std::string OUTBOUND_RINGING_STARTED_EVENT_PAYLOAD =
    R"({
        "callId": "1234"
    })";

static const std::string OUTBOUND_RINGING_STARTED_CONTEXT =
    R"({
        "device": {
            "connectionState": "CONNECTED"
        },
        "configuration": {
            "callingFeature": [
                {
                    "OVERRIDE_RINGTONE_SUPPORTED": false
                }
            ]
        },
        "allCalls": [
            {
                "callId": "1234",
                "callState": "OUTBOUND_RINGING"
            }
        ],
        "currentCall": {
            "callId": "1234"
        }
    })";

static const std::string CALL_RECEIVED_EVENT_NAME = "CallReceived";

static const std::string CALL_RECEIVED_EVENT_PAYLOAD =
    R"({
        "allCalls": [
            {
                "callId": "1234",
                "callState": "INVITED"
            }
        ],
        "receivedCall": {
                "callId": "1234"
        }
    })";

static const std::string CALL_RECEIVED_CONTEXT =
    R"({
        "device": {
            "connectionState": "CONNECTED"
        },
        "configuration": {
            "callingFeature": [
                {
                    "OVERRIDE_RINGTONE_SUPPORTED": false
                }
            ]
        },
        "allCalls": [
            {
                "callId": "1234",
                "callState": "INVITED"
            }
        ],
        "currentCall": {
            "callId": "1234"
        }
    })";

static const std::string INBOUND_RINGING_STARTED_EVENT_NAME = "InboundRingingStarted";

static const std::string INBOUND_RINGING_STARTED_EVENT_PAYLOAD =
    R"({
        "callId": "1234"
    })";

static const std::string INBOUND_RINGING_STARTED_CONTEXT =
    R"({
        "device": {
            "connectionState": "CONNECTED"
        },
        "configuration": {
            "callingFeature": [
                {
                    "OVERRIDE_RINGTONE_SUPPORTED": false
                }
            ]
        },
        "allCalls": [
            {
                "callId": "1234",
                "callState": "INBOUND_RINGING"
            }
        ],
        "currentCall": {
            "callId": "1234"
        }
    })";

static std::chrono::milliseconds TIMEOUT(1000);

static const std::string NAMESPACE{"Alexa.Comms.PhoneCallController"};

static const std::string UNKNOWN_DIRECTIVE{"Unknown"};

static const alexaClientSDK::avsCommon::avs::NamespaceAndName DIAL{NAMESPACE, "Dial"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName REDIAL{NAMESPACE, "Redial"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName ANSWER{NAMESPACE, "Answer"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName STOP{NAMESPACE, "Stop"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName SENDDTMF{NAMESPACE, "SendDTMF"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName PCC_STATE{NAMESPACE, "PhoneCallControllerState"};

static const std::string MESSAGE_ID("messageId");

MATCHER_P(matchJson, expectedJson, "") {
    try {
        nlohmann::json expected = nlohmann::json::parse(expectedJson);
        nlohmann::json actual = nlohmann::json::parse(arg);
        return expected == actual;
    } catch (nlohmann::json::exception& e) {
        return false;
    }
}

MATCHER_P(matchMessage, expected, "") {
    std::string expectedJson = expected->getJsonContent();
    std::string actualJson = arg->getJsonContent();

    try {
        nlohmann::json expected = nlohmann::json::parse(expectedJson);
        nlohmann::json actual = nlohmann::json::parse(actualJson);

        nlohmann::json expectedHeader = expected["event"]["header"];
        nlohmann::json actualHeader = actual["event"]["header"];
        if (expectedHeader["namespace"] != actualHeader["namespace"] ||
            expectedHeader["name"] != actualHeader["name"]) {
            return false;
        }

        nlohmann::json expectedContext = expected["context"];
        nlohmann::json actualContext = actual["context"];
        if (expectedContext != actualContext) {
            return false;
        }

        nlohmann::json expectedPayload = expected["payload"];
        nlohmann::json actualPayload = actual["payload"];
        if (expectedPayload != actualPayload) {
            return false;
        }

        return true;
    } catch (nlohmann::json::exception& e) {
        return false;
    }
}

class MockGui : public aace::engine::phoneCallController::PhoneCallControllerInterface {
public:
    MOCK_METHOD1(dial, bool(const std::string& payload));
    MOCK_METHOD1(redial, bool(const std::string& payload));
    MOCK_METHOD1(answer, void(const std::string& payload));
    MOCK_METHOD1(stop, void(const std::string& payload));
    MOCK_METHOD1(playRingtone, void(const std::string& payload));
    MOCK_METHOD1(sendDTMF, void(const std::string& payload));
};

class PhoneCallControllerCapabilityAgentTest : public ::testing::Test {
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
        m_mockFocusManager =
            std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>>();
        m_mockGui = std::make_shared<testing::StrictMock<MockGui>>();
        EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_))
            .WillOnce(testing::Return(alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS));
        EXPECT_CALL(*m_mockFocusManager, releaseChannel(testing::_, testing::_))
            .Times(testing::AtLeast(1))
            .WillRepeatedly(testing::InvokeWithoutArgs([this] {
                auto tempFuture = std::promise<bool>();
                m_releaseChannelPromise.swap(tempFuture);
                m_releaseChannelPromise.set_value(true);
                return m_releaseChannelPromise.get_future();
            }));
        m_capAgent = aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::create(
            m_mockGui, m_mockContextManager, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager);
    }
    void TearDown() override {
        m_capAgent->shutdown();
    }

    std::string generatePayload(std::string callId);
    std::string getContextWithHeader(const std::string& pccContextPayload);
    void setupExpectedSendMessage(
        const std::string& expectedContext,
        const std::string& expectedEvent,
        const std::string& eventName,
        int times);
    void setupExpectedContextUpdate(const std::string& expectedContext, int times);
    void wakeOnSetCompleted();

    PhoneCallControllerCapabilityAgentTest() :
            m_wakeSetCompletedPromise{}, m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()} {
    }

    std::promise<void> m_wakeSetCompletedPromise;
    std::future<void> m_wakeSetCompletedFuture;
    std::promise<bool> m_releaseChannelPromise;

    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent> m_capAgent;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>
        m_mockContextManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>
        m_mockExceptionSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>>
        m_mockMessageSender;
    std::unique_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveHandlerResult>>
        m_mockDirectiveHandlerResult;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>>
        m_mockFocusManager;
    std::shared_ptr<testing::StrictMock<MockGui>> m_mockGui;
};

std::string PhoneCallControllerCapabilityAgentTest::generatePayload(std::string callId) {
    std::string payload =
        "{"
        "\"callId\":\"" +
        callId +
        "\","
        "\"callee\":{"
        "\"details\":\"DETAILS\","
        "\"defaultAddresses\": {"
        "\"protocol\": \"PROTOCOL\","
        "\"format\": \"FORMAT\","
        "\"value\": \"VALUE\""
        "},"
        "\"alternativeAddresses\": [{"
        "\"protocol\": \"PROTOCOL\","
        "\"format\": \"FORMAT\","
        "\"value\": \"VALUE\""
        "}]"
        "}"
        "}";
    return payload;
}

std::string PhoneCallControllerCapabilityAgentTest::getContextWithHeader(const std::string& pccContextPayload) {
    std::string contextWithHeader =
        R"([
            {
                "header": {
                    "namespace": "Alexa.Comms.PhoneCallController",
                    "name": "PhoneCallControllerState"
                },
                "payload": <payload>
            }]
        )";
    contextWithHeader = std::regex_replace(contextWithHeader, std::regex("<payload>"), pccContextPayload);
    return contextWithHeader;
}

void PhoneCallControllerCapabilityAgentTest::setupExpectedSendMessage(
    const std::string& expectedContext,
    const std::string& expectedEvent,
    const std::string& eventName,
    int times) {
    auto expectedJsonEventString = alexaClientSDK::avsCommon::avs::buildJsonEventString(
        NAMESPACE, eventName, "", expectedEvent, getContextWithHeader(expectedContext));
    auto expectedMessageRequest =
        std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(expectedJsonEventString.second);
    EXPECT_CALL(*m_mockMessageSender, sendMessage(matchMessage(expectedMessageRequest))).Times(times);
}

void PhoneCallControllerCapabilityAgentTest::setupExpectedContextUpdate(const std::string& expectedContext, int times) {
    EXPECT_CALL(
        *m_mockContextManager,
        setState(PCC_STATE, matchJson(expectedContext), alexaClientSDK::avsCommon::avs::StateRefreshPolicy::NEVER, 0))
        .Times(times)
        .WillRepeatedly(testing::Return(alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS));
}

void PhoneCallControllerCapabilityAgentTest::wakeOnSetCompleted() {
    m_wakeSetCompletedPromise.set_value();
}

TEST_F(PhoneCallControllerCapabilityAgentTest, create) {
    EXPECT_NE(nullptr, m_capAgent);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, createWithNullContextManager) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent> capAgent;
    capAgent = aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::create(
        m_mockGui, nullptr, m_mockExceptionSender, m_mockMessageSender, m_mockFocusManager);
    EXPECT_EQ(nullptr, capAgent);
}
TEST_F(PhoneCallControllerCapabilityAgentTest, createWithNullExceptionSender) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent> capAgent;
    capAgent = aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::create(
        m_mockGui, m_mockContextManager, nullptr, m_mockMessageSender, m_mockFocusManager);
    EXPECT_EQ(nullptr, capAgent);
}
TEST_F(PhoneCallControllerCapabilityAgentTest, createWithNullMessageSender) {
    std::shared_ptr<aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent> capAgent;
    capAgent = aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::create(
        m_mockGui, m_mockContextManager, m_mockExceptionSender, nullptr, m_mockFocusManager);
    EXPECT_EQ(nullptr, capAgent);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testUnknownDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(NAMESPACE, UNKNOWN_DIRECTIVE, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create("", avsMessageHeader, "", attachmentManager, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));

    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testDialDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(DIAL.nameSpace, DIAL.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create("", avsMessageHeader, DIAL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, dial(DIAL_PAYLOAD)).Times(testing::Exactly(1)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(2));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist("CALLID"), true);
    ASSERT_EQ(
        m_capAgent->getCallState("CALLID"),
        aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::CallState::IDLE);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testDialDirectiveReturnFalse) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(DIAL.nameSpace, DIAL.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create("", avsMessageHeader, DIAL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, dial(DIAL_PAYLOAD)).Times(testing::Exactly(1)).WillOnce(testing::Return(false));
    EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(2));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist("CALLID"), false);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testDialDirectiveNoCallId) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(DIAL.nameSpace, DIAL.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, DIAL_PAYLOAD_NO_CALLID, attachmentManager, "");

    EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist("CALLID"), false);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testDialDirectiveBadFormat) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(DIAL.nameSpace, DIAL.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, DIAL_PAYLOAD_BAD_FORMAT, attachmentManager, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));

    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist("CALLID"), false);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testRedialDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(REDIAL.nameSpace, REDIAL.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, REDIAL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, redial(REDIAL_PAYLOAD)).Times(testing::Exactly(1)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(2));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist("CALLID"), true);
    ASSERT_EQ(
        m_capAgent->getCallState("CALLID"),
        aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::CallState::IDLE);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testRedialDirectiveReturnFalse) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(REDIAL.nameSpace, REDIAL.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, REDIAL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, redial(REDIAL_PAYLOAD)).Times(testing::Exactly(1)).WillOnce(testing::Return(false));
    EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(2));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist("CALLID"), false);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testAnswerDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(ANSWER.nameSpace, ANSWER.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, ANSWER_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, answer(ANSWER_PAYLOAD)).Times(testing::Exactly(1));
    EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testStopDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(STOP.nameSpace, STOP.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create("", avsMessageHeader, STOP_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    EXPECT_CALL(*m_mockGui, stop(STOP_PAYLOAD)).Times(testing::Exactly(1));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testSENDDTMFDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(
        SENDDTMF.nameSpace, SENDDTMF.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, SENDDTMF_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockContextManager, setState(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    EXPECT_CALL(*m_mockGui, sendDTMF(SENDDTMF_PAYLOAD)).Times(testing::Exactly(1));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testCallFailedInvalidCallId) {
    setupExpectedContextUpdate(PHONE_CONNECTED_CONTEXT, 1);
    EXPECT_CALL(*m_mockMessageSender, sendMessage(testing::_)).Times(0);

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->callFailed(
        TEST_CALL_ID, aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::OTHER, "");

    ASSERT_EQ(m_capAgent->callExist(TEST_CALL_ID), false);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testCallFailedNoAnswer) {
    auto attachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(DIAL.nameSpace, DIAL.name, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, generatePayload(TEST_CALL_ID), attachmentManager, "");

    EXPECT_CALL(*m_mockGui, dial(testing::_)).Times(testing::Exactly(1)).WillOnce(testing::Return(true));
    setupExpectedContextUpdate(PHONE_CONNECTED_CONTEXT, 3);  // One expect is for callFailed
    setupExpectedContextUpdate(DIAL_STARTED_CONTEXT, 1);
    EXPECT_CALL(*m_mockFocusManager, acquireChannel(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1))
        .WillRepeatedly(testing::Return(true));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(directive->getMessageId());

    setupExpectedSendMessage(DIAL_STARTED_CONTEXT, DIAL_STARTED_EVENT_PAYLOAD, DIAL_STARTED_EVENT_NAME, 1);
    m_capAgent->callStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::DIALING, TEST_CALL_ID, "");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    setupExpectedSendMessage(CALL_FAILED_CONTEXT, CALL_FAILED_NO_ANSWER_EVENT_PAYLOAD, CALL_FAILED_EVENT_NAME, 1);
    m_capAgent->callFailed(
        TEST_CALL_ID, aace::phoneCallController::PhoneCallControllerEngineInterface::CallError::NO_ANSWER, "TEST");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist(TEST_CALL_ID), false);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testConnectionStateChangedConnected) {
    setupExpectedContextUpdate(PHONE_CONNECTED_CONTEXT, 1);
    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testGUIStopActiveCall) {
    setupExpectedContextUpdate(PHONE_CONNECTED_CONTEXT, 2);  // One expect is for callTerminated
    setupExpectedSendMessage(CALL_TERMINATED_CONTEXT, CALL_TERMINATED_EVENT_PAYLOAD, CALL_TERMINATED_EVENT_NAME, 1);

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->callStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::IDLE, TEST_CALL_ID, "");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist(TEST_CALL_ID), false);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testGUIStartCall) {
    setupExpectedContextUpdate(PHONE_CONNECTED_CONTEXT, 1);
    setupExpectedContextUpdate(CALL_ACTIVATED_CONTEXT, 1);
    setupExpectedSendMessage(CALL_ACTIVATED_CONTEXT, CALL_ACTIVATED_EVENT_PAYLOAD, CALL_ACTIVATED_EVENT_NAME, 1);
    EXPECT_CALL(*m_mockFocusManager, acquireChannel(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1))
        .WillOnce(testing::Return(true));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->callStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::ACTIVE, TEST_CALL_ID, "");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist(TEST_CALL_ID), true);
    ASSERT_EQ(
        m_capAgent->getCallState(TEST_CALL_ID),
        aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::CallState::ACTIVE);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testCallStateChangedOutboundRinging) {
    setupExpectedContextUpdate(PHONE_CONNECTED_CONTEXT, 1);
    setupExpectedContextUpdate(OUTBOUND_RINGING_STARTED_CONTEXT, 1);
    setupExpectedSendMessage(
        OUTBOUND_RINGING_STARTED_CONTEXT,
        OUTBOUND_RINGING_STARTED_EVENT_PAYLOAD,
        OUTBOUND_RINGING_STARTED_EVENT_NAME,
        1);
    EXPECT_CALL(*m_mockFocusManager, acquireChannel(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1))
        .WillOnce(testing::Return(true));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->callStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::OUTBOUND_RINGING, TEST_CALL_ID, "");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist(TEST_CALL_ID), true);
    ASSERT_EQ(
        m_capAgent->getCallState(TEST_CALL_ID),
        aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::CallState::OUTBOUND_RINGING);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testCallStateChangedCallReceived) {
    setupExpectedContextUpdate(PHONE_CONNECTED_CONTEXT, 1);
    setupExpectedContextUpdate(CALL_RECEIVED_CONTEXT, 1);
    setupExpectedSendMessage(CALL_RECEIVED_CONTEXT, CALL_RECEIVED_EVENT_PAYLOAD, CALL_RECEIVED_EVENT_NAME, 1);
    EXPECT_CALL(*m_mockFocusManager, acquireChannel(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1))
        .WillOnce(testing::Return(true));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->callStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::CALL_RECEIVED, TEST_CALL_ID, "");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist(TEST_CALL_ID), true);
    ASSERT_EQ(
        m_capAgent->getCallState(TEST_CALL_ID),
        aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::CallState::INVITED);
}

TEST_F(PhoneCallControllerCapabilityAgentTest, testCallStateChangedInboundRinging) {
    setupExpectedContextUpdate(PHONE_CONNECTED_CONTEXT, 1);
    setupExpectedContextUpdate(INBOUND_RINGING_STARTED_CONTEXT, 1);
    setupExpectedSendMessage(
        INBOUND_RINGING_STARTED_CONTEXT, INBOUND_RINGING_STARTED_EVENT_PAYLOAD, INBOUND_RINGING_STARTED_EVENT_NAME, 1);
    EXPECT_CALL(*m_mockFocusManager, acquireChannel(testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1))
        .WillOnce(testing::Return(true));

    m_capAgent->connectionStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED);
    m_capAgent->callStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallState::INBOUND_RINGING, TEST_CALL_ID, "");
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);

    ASSERT_EQ(m_capAgent->callExist(TEST_CALL_ID), true);
    ASSERT_EQ(
        m_capAgent->getCallState(TEST_CALL_ID),
        aace::engine::phoneCallController::PhoneCallControllerCapabilityAgent::CallState::INBOUND_RINGING);
}

}  // namespace phoneCallController
}  // namespace unit
}  // namespace test
}  // namespace aace
