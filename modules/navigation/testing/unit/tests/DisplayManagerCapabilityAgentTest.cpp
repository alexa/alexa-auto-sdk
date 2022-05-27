/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <future>

#include <AVSCommon/AVS/EventBuilder.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveHandlerResult.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>

#include <AACE/Engine/Navigation/DisplayManagerCapabilityAgent.h>
#include <AACE/Test/Unit/AVS/MockAttachmentManager.h>

namespace aace {
namespace test {
namespace unit {
namespace navigation {

using namespace aace::engine::navigation;

static std::chrono::milliseconds TIMEOUT(1500);

/// The namespace for @c DisplayManager interface.
static const std::string DISPLAY_MANAGER_NAMESPACE = "Navigation.DisplayManager";

/// The @c ControlDisplay directive name.
static const std::string CONTROL_DISPLAY = "ControlDisplay";

/// The @c ShowAlternativeRoutes directive name.
static const std::string SHOW_ALTERNATIVE_ROUTES = "ShowAlternativeRoutes";

/// @c ControlDisplaySucceeded event name.
static const std::string CONTROL_DISPLAY_SUCCEEDED = "ControlDisplaySucceeded";

/// @c ShowAlternativeRoutesSucceeded event name.
static const std::string SHOW_ALTERNATIVE_ROUTES_SUCCEEDED = "ShowAlternativeRoutesSucceeded";

/// @c ControlDisplayFailed event name.
static const std::string CONTROL_DISPLAY_FAILED = "ControlDisplayFailed";

/// @c ShowAlternativeRoutesFailed event name.
static const std::string SHOW_ALTERNATIVE_ROUTES_FAILED = "ShowAlternativeRoutesFailed";

/// Message ID used for mocked directives or events.
static const std::string MESSAGE_ID_TEST = "1234";

/// Sample @c ControlDisplay directive payload for "Show me the whole route"
static const std::string DISPLAY_ROUTE_OVERVIEW_DIRECTIVE_PAYLOAD =
    R"({
        "mode": "SHOW_ROUTE_OVERVIEW"
    })";

/// Sample @c ControlDisplaySucceeded event payload for "Show me the whole route"
static const std::string DISPLAY_ROUTE_OVERVIEW_SUCCESS_PAYLOAD =
    R"({
        "mode": "SHOW_ROUTE_OVERVIEW"
    })";

/// Sample @c ControlDisplay directive payload for "Zoom out"
static const std::string DISPLAY_ZOOM_OUT_DIRECTIVE_PAYLOAD =
    R"({
        "mode": "ZOOM_OUT"
    })";

/// Sample @c ControlDisplayFailed event payload for "Zoom out"
static const std::string DISPLAY_ZOOM_OUT_NOT_SUPPORTED_PAYLOAD =
    R"({
        "mode": "ZOOM_OUT",
        "code", "NOT_SUPPORTED",
        "description": "max zoom out reached"

    })";

/// Sample @c ControlDisplayFailed event payload for "Orient the map with north up", optional description omitted
static const std::string DISPLAY_ORIENT_NORTH_NOT_ALLOWED_PAYLOAD_MINIMAL =
    R"({
        "mode": "ORIENT_NORTH",
        "code", "NOT_ALLOWED"
    })";

/// Sample @c ShowAlternativeRoutes directive payload for "Show me a faster route"
static const std::string SHOW_ALT_ROUTES_FASTER_DIRECTIVE_PAYLOAD =
    R"({
        "inquiryType": "SHORTER_TIME"
    })";

/// Sample @c ShowAlternativeRoutesSucceeded payload for "Show me a faster route"
static const std::string SHOW_ALT_ROUTES_FASTER_SUCCESS_PAYLOAD =
    R"({
        "inquiryType": "SHORTER_TIME",
        "alternateRoute": {
            "labels": ["US-101 N", "CA-237 E"],
            "savings": [
                {
                    "type": "TIME",
                    "amount": 12,
                    "unit": "MINUTE"
                }
            ]              
        }
    })";

/// Sample @c ShowAlternativeRoutes directive payload for "Show alternate routes"
static const std::string SHOW_ALT_ROUTES_DEFAULT_DIRECTIVE_PAYLOAD =
    R"({
        "inquiryType": "DEFAULT"
    })";

/// Sample @c ShowAlternativeRoutesFailed payload for "Show alternate routes"
static const std::string SHOW_ALT_ROUTES_DEFAULT_NONE_FOUND_PAYLOAD =
    R"({
        "inquiryType": "DEFAULT",
        "code": "NO_ROUTES_FOUND",
        "description": "already on best route"
    })";

/// Sample @c ShowAlternativeRoutesSucceeded payload for "Show a shorter route", optional savings info omitted
static const std::string SHOW_ALT_ROUTES_SHORTER_DISTANCE_SUCCESS_PAYLOAD_MINIMAL =
    R"({
        "inquiryType": "SHORTER_DISTANCE",
        "alternateRoute": {
            "labels": ["Main street"]
        }
    })";

/// Sample @c ShowAlternativeRoutesFailed payload for "Show a faster route", optional description omitted
static const std::string SHOW_ALT_ROUTES_FASTER_NOT_NAVIGATING_PAYLOAD_MINIMAL =
    R"({
        "inquiryType": "SHORTER_TIME",
        "code": "NOT_NAVIGATING"
    })";

/**
 * Compare two AVS events (as serialized JSON strings) for equality. Returns @c true if the following are the same
 * across the two events:
 *  - Header namespace
 *  - Header name
 *  - Context object (order of attributes is irrelevant)
 *  - Payload object (order of attributes is irrelevant)
 */
MATCHER_P(matchEvent, expected, "") {
    std::string expectedJson = expected->getJsonContent();
    std::string actualJson = arg->getJsonContent();
    std::cout << "Expected message JSON: " << expectedJson << "\n";
    std::cout << "Actual message JSON: " << actualJson << "\n";

    try {
        nlohmann::json expected = nlohmann::json::parse(expectedJson);
        nlohmann::json actual = nlohmann::json::parse(actualJson);

        nlohmann::json expectedHeader = expected["event"]["header"];
        nlohmann::json actualHeader = actual["event"]["header"];
        if (expectedHeader["namespace"] != actualHeader["namespace"]) {
            std::cout << "Header namespace doesn't match \n";
        }
        if (expectedHeader["name"] != actualHeader["name"]) {
            std::cout << "Header name doesn't match \n";
            return false;
        }

        nlohmann::json expectedContext = expected["context"];
        nlohmann::json actualContext = actual["context"];
        if (expectedContext != actualContext) {
            std::cout << "Context doesn't match \n";
            return false;
        }

        nlohmann::json expectedPayload = expected["payload"];
        nlohmann::json actualPayload = actual["payload"];
        if (expectedPayload != actualPayload) {
            std::cout << "Payload doesn't match \n";
            return false;
        }
        return true;
    } catch (nlohmann::json::exception& e) {
        std::cout << "JSON parse error \n";
        return false;
    }
}

class MockDisplayHandler : public aace::engine::navigation::DisplayHandlerInterface {
public:
    MOCK_METHOD1(controlDisplay, void(aace::engine::navigation::DisplayMode controlType));
    MOCK_METHOD1(showAlternativeRoutes, void(aace::engine::navigation::AlternativeRoutesQueryType queryType));
};

class DisplayManagerCapabilityAgentTest : public ::testing::Test {
public:
    DisplayManagerCapabilityAgentTest();

    void SetUp() override;
    void TearDown() override;

    /**
     * Send a directive with the specified info to @c m_capabilityAgent.
     */
    void sendDirective(
        const std::string& directiveNamespace,
        const std::string& name,
        const std::string& payload,
        const std::string& messageId);

    /**
     * Set an expect condition that @c m_mockMessageSender @c sendMessage() will be invoked with the specified event
     * details.
     */
    void setupExpectedSendMessage(
        const std::string& expectedContext,
        const std::string& expectedEventPayload,
        const std::string& expectedEventName,
        const ::std::string& expectedNamespace = DISPLAY_MANAGER_NAMESPACE,
        int times = 1);

    /**
     * Invoke when @c sendMessage is called on @c m_mockMessageSender.
     */
    void wakeOnSendMessage();

    /// Promise to be fulfilled when @c sendMessage is called.
    std::promise<void> m_wakeSendMessagePromise;

    /// Future to notify when @c sendMessage is called.
    std::future<void> m_wakeSendMessageFuture;

    std::shared_ptr<aace::engine::navigation::DisplayManagerCapabilityAgent> m_capabilityAgent;
    std::shared_ptr<testing::StrictMock<MockDisplayHandler>> m_mockDisplayHandler;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>
        m_mockExceptionSender;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>
        m_mockContextManager;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>>
        m_mockMessageSender;
    std::shared_ptr<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>> m_mockAttachmentManager;
    std::unique_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveHandlerResult>>
        m_mockDirectiveHandlerResult;
};

DisplayManagerCapabilityAgentTest::DisplayManagerCapabilityAgentTest() :
        m_wakeSendMessagePromise{}, m_wakeSendMessageFuture{m_wakeSendMessagePromise.get_future()} {
}

void DisplayManagerCapabilityAgentTest::SetUp() {
    m_mockDisplayHandler = std::make_shared<testing::StrictMock<MockDisplayHandler>>();
    m_mockExceptionSender = std::make_shared<
        testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
    m_mockMessageSender =
        std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>>();
    m_mockContextManager =
        std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
    m_capabilityAgent = aace::engine::navigation::DisplayManagerCapabilityAgent::create(
        m_mockDisplayHandler, m_mockExceptionSender, m_mockMessageSender, m_mockContextManager);
    m_mockAttachmentManager = std::make_shared<testing::StrictMock<aace::test::unit::avs::MockAttachmentManager>>();
    m_mockDirectiveHandlerResult = std::unique_ptr<
        testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveHandlerResult>>();
}

void DisplayManagerCapabilityAgentTest::TearDown() {
    m_capabilityAgent->shutdown();
}

void DisplayManagerCapabilityAgentTest::sendDirective(
    const std::string& directiveNamespace,
    const std::string& name,
    const std::string& payload,
    const std::string& messageId) {
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(directiveNamespace, name, messageId);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create(
            "", avsMessageHeader, payload, m_mockAttachmentManager, "");

    m_capabilityAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capabilityAgent->CapabilityAgent::handleDirective(messageId);
}

void DisplayManagerCapabilityAgentTest::setupExpectedSendMessage(
    const std::string& expectedContext,
    const std::string& expectedEventPayload,
    const std::string& expectedEventName,
    const ::std::string& expectedNamespace,
    int times) {
    auto expectedJsonEventString = alexaClientSDK::avsCommon::avs::buildJsonEventString(
        expectedNamespace, expectedEventName, "", expectedEventPayload, expectedContext);
    auto expectedMessageRequest =
        std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(expectedJsonEventString.second);
    EXPECT_CALL(*m_mockMessageSender, sendMessage(matchEvent(expectedMessageRequest)))
        .Times(times)
        .WillOnce(InvokeWithoutArgs(this, &DisplayManagerCapabilityAgentTest::wakeOnSendMessage));
}

void DisplayManagerCapabilityAgentTest::wakeOnSendMessage() {
    m_wakeSendMessagePromise.set_value();
}

TEST_F(DisplayManagerCapabilityAgentTest, testCreateSuccess) {
    EXPECT_NE(nullptr, m_capabilityAgent);
}

TEST_F(DisplayManagerCapabilityAgentTest, testCreateFailsNullDisplayHandler) {
    auto invalid = DisplayManagerCapabilityAgent::create(
        nullptr, m_mockExceptionSender, m_mockMessageSender, m_mockContextManager);
    EXPECT_EQ(nullptr, invalid);
}

TEST_F(DisplayManagerCapabilityAgentTest, testCreateFailsNullExceptionSender) {
    auto invalid =
        DisplayManagerCapabilityAgent::create(m_mockDisplayHandler, nullptr, m_mockMessageSender, m_mockContextManager);
    EXPECT_EQ(nullptr, invalid);
}

TEST_F(DisplayManagerCapabilityAgentTest, testCreateFailsNullMessageSender) {
    auto invalid = DisplayManagerCapabilityAgent::create(
        m_mockDisplayHandler, m_mockExceptionSender, nullptr, m_mockContextManager);
    EXPECT_EQ(nullptr, invalid);
}

TEST_F(DisplayManagerCapabilityAgentTest, testCreateFailsNullContextManager) {
    auto invalid = DisplayManagerCapabilityAgent::create(
        m_mockDisplayHandler, m_mockExceptionSender, m_mockMessageSender, nullptr);
    EXPECT_EQ(nullptr, invalid);
}

/**
 * Send a @c ControlDisplay directive to the capability agent and verify the following:
 *  - Capability agent invokes @c DisplayHandlerInterface::controlDisplay() with the correct display mode
 *  - When the client calls back @c DisplayManagerCapabilityAgent::controlDisplaySucceeded(), the capability
 *    agent sends a @c ControlDisplaySucceeded event with payload matching the info specified in the callback
 */
TEST_F(DisplayManagerCapabilityAgentTest, testDisplayRouteOverviewSuccess) {
    EXPECT_CALL(*m_mockDisplayHandler, controlDisplay(DisplayMode::SHOW_ROUTE_OVERVIEW)).Times(testing::Exactly(1));
    sendDirective(
        DISPLAY_MANAGER_NAMESPACE, CONTROL_DISPLAY, DISPLAY_ROUTE_OVERVIEW_DIRECTIVE_PAYLOAD, MESSAGE_ID_TEST);

    setupExpectedSendMessage("", DISPLAY_ROUTE_OVERVIEW_SUCCESS_PAYLOAD, CONTROL_DISPLAY_SUCCEEDED);
    m_capabilityAgent->controlDisplaySucceeded(DisplayMode::SHOW_ROUTE_OVERVIEW);
    ASSERT_EQ(std::future_status::ready, m_wakeSendMessageFuture.wait_for(TIMEOUT));
}

/**
 * Send a @c ControlDisplay directive to the capability agent and verify the following:
 *  - Capability agent invokes @c DisplayHandlerInterface::controlDisplay() with the correct display mode
 *  - When the client calls back @c DisplayManagerCapabilityAgent::controlDisplayFailed(), the capability
 *    agent sends a @c ControlDisplayFailed event with payload matching the info specified in the callback
 */
TEST_F(DisplayManagerCapabilityAgentTest, testZoomOutFailed) {
    EXPECT_CALL(*m_mockDisplayHandler, controlDisplay(DisplayMode::ZOOM_OUT)).Times(testing::Exactly(1));
    sendDirective(DISPLAY_MANAGER_NAMESPACE, CONTROL_DISPLAY, DISPLAY_ZOOM_OUT_DIRECTIVE_PAYLOAD, MESSAGE_ID_TEST);

    setupExpectedSendMessage("", DISPLAY_ZOOM_OUT_NOT_SUPPORTED_PAYLOAD, CONTROL_DISPLAY_FAILED);
    m_capabilityAgent->controlDisplayFailed(
        DisplayMode::ZOOM_OUT, DisplayControlError::NOT_SUPPORTED, "max zoom out reached");
    ASSERT_EQ(std::future_status::ready, m_wakeSendMessageFuture.wait_for(TIMEOUT));
}

/**
 * Verifies that when the client calls back @c DisplayManagerCapabilityAgent::controlDisplayFailed() with
 * optional fields omitted in parameters, the capability agent omits the corresponding fields in the
 * @c ControlDisplayFailed event payload
 */
TEST_F(DisplayManagerCapabilityAgentTest, testControlDisplayFailedOmitsOptionalFields) {
    setupExpectedSendMessage("", DISPLAY_ORIENT_NORTH_NOT_ALLOWED_PAYLOAD_MINIMAL, CONTROL_DISPLAY_FAILED);
    // Leave out optional description
    m_capabilityAgent->controlDisplayFailed(DisplayMode::ORIENT_NORTH, DisplayControlError::NOT_ALLOWED);
    ASSERT_EQ(std::future_status::ready, m_wakeSendMessageFuture.wait_for(TIMEOUT));
}

/**
 * Send a @c ShowAlternativeRoutes directive to the capability agent and verify the following:
 *  - Capability agent invokes @c DisplayHandlerInterface::showAlternativeRoutes() with the correct query type
 *  - When the client calls back @c DisplayManagerCapabilityAgent::showAlternativeRoutesSucceeded(), the capability
 *    agent sends a @c ShowAlternativeRoutesSucceeded event with payload matching the info specified in the callback
 */
TEST_F(DisplayManagerCapabilityAgentTest, testShowFasterAltRouteSuccess) {
    EXPECT_CALL(*m_mockDisplayHandler, showAlternativeRoutes(AlternativeRoutesQueryType::SHORTER_TIME))
        .Times(testing::Exactly(1));
    sendDirective(
        DISPLAY_MANAGER_NAMESPACE, SHOW_ALTERNATIVE_ROUTES, SHOW_ALT_ROUTES_FASTER_DIRECTIVE_PAYLOAD, MESSAGE_ID_TEST);

    setupExpectedSendMessage("", SHOW_ALT_ROUTES_FASTER_SUCCESS_PAYLOAD, SHOW_ALTERNATIVE_ROUTES_SUCCEEDED);
    AlternateRoute altRoute =
        AlternateRoute({"US-101 N", "CA-237 E"}, {RouteSavings(RouteSavingsType::TIME, 12, SavingsUnit::MINUTE)});
    m_capabilityAgent->showAlternativeRoutesSucceeded(AlternativeRoutesQueryType::SHORTER_TIME, altRoute);
    ASSERT_EQ(std::future_status::ready, m_wakeSendMessageFuture.wait_for(TIMEOUT));
}

/**
 * Send a @c ShowAlternativeRoutes directive to the capability agent and verify the following:
 *  - Capability agent invokes @c DisplayHandlerInterface::showAlternativeRoutes() with the correct query type
 *  - When the client calls back @c DisplayManagerCapabilityAgent::showAlternativeRoutesFailed(), the capability
 *    agent sends a @c ShowAlternativeRoutesFailed event with payload matching the info specified in the callback
 */
TEST_F(DisplayManagerCapabilityAgentTest, testShowDefaultAltRouteFailure) {
    EXPECT_CALL(*m_mockDisplayHandler, showAlternativeRoutes(AlternativeRoutesQueryType::DEFAULT))
        .Times(testing::Exactly(1));
    sendDirective(
        DISPLAY_MANAGER_NAMESPACE, SHOW_ALTERNATIVE_ROUTES, SHOW_ALT_ROUTES_DEFAULT_DIRECTIVE_PAYLOAD, MESSAGE_ID_TEST);

    setupExpectedSendMessage("", SHOW_ALT_ROUTES_DEFAULT_NONE_FOUND_PAYLOAD, SHOW_ALTERNATIVE_ROUTES_FAILED);
    m_capabilityAgent->showAlternativeRoutesFailed(
        AlternativeRoutesQueryType::DEFAULT, AlternativeRoutesQueryError::NO_ROUTES_FOUND, "already on best route");
    ASSERT_EQ(std::future_status::ready, m_wakeSendMessageFuture.wait_for(TIMEOUT));
}

/**
 * Verifies that when the client calls back @c DisplayManagerCapabilityAgent::showAlternativeRoutesSucceeded() with
 * optional fields omitted in parameters, the capability agent omits the corresponding fields in the
 * @c ShowAlternativeRoutesSucceeded event payload
 */
TEST_F(DisplayManagerCapabilityAgentTest, testShowAltRouteSuccessOmitsOptionalFields) {
    setupExpectedSendMessage(
        "", SHOW_ALT_ROUTES_SHORTER_DISTANCE_SUCCESS_PAYLOAD_MINIMAL, SHOW_ALTERNATIVE_ROUTES_SUCCEEDED);
    // Leave out optional route savings details
    AlternateRoute altRoute = AlternateRoute({"Main street"});
    m_capabilityAgent->showAlternativeRoutesSucceeded(AlternativeRoutesQueryType::SHORTER_DISTANCE, altRoute);
    ASSERT_EQ(std::future_status::ready, m_wakeSendMessageFuture.wait_for(TIMEOUT));
}

/**
 * Verifies that when the client calls back @c DisplayManagerCapabilityAgent::showAlternativeRoutesFailed() with
 * optional fields omitted in parameters, the capability agent omits the corresponding fields in the
 * @c ShowAlternativeRoutesFailed event payload
 */
TEST_F(DisplayManagerCapabilityAgentTest, testShowAltRouteFailureOmitsOptionalFields) {
    setupExpectedSendMessage("", SHOW_ALT_ROUTES_FASTER_NOT_NAVIGATING_PAYLOAD_MINIMAL, SHOW_ALTERNATIVE_ROUTES_FAILED);
    // Leave out optional description
    m_capabilityAgent->showAlternativeRoutesFailed(
        AlternativeRoutesQueryType::SHORTER_TIME, AlternativeRoutesQueryError::NOT_NAVIGATING);
    ASSERT_EQ(std::future_status::ready, m_wakeSendMessageFuture.wait_for(TIMEOUT));
}

}  // namespace navigation
}  // namespace unit
}  // namespace test
}  // namespace aace