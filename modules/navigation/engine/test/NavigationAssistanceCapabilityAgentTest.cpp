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
#include <future>

#include "MockAttachmentManager.h"
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveHandlerResult.h>
#include "AVSCommon/SDKInterfaces/test/MockContextManager.h"

#include "AACE/Engine/Navigation/NavigationAssistanceCapabilityAgent.h"
#include "AACE/Test/Alexa/AlexaTestHelper.h"

namespace aace {
namespace test {
namespace unit {

static std::chrono::milliseconds TIMEOUT(1000);

static const std::string NAMESPACE{"Navigation.Assistance"};

static const std::string UNKNOWN_DIRECTIVE{"Unknown"};

static const alexaClientSDK::avsCommon::avs::NamespaceAndName ANNOUNCE_MANEUVER{NAMESPACE, "AnnounceManeuver"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName ANNOUNCE_ROAD_REGULATION{NAMESPACE,
                                                                                       "AnnounceRoadRegulation"};

static const std::string MESSAGE_ID("messageId");

class TestNavigationHandler : public aace::engine::navigation::NavigationHandlerInterface {
public:
    MOCK_METHOD0(cancelNavigation, void());
    MOCK_METHOD0(getNavigationState, std::string());
    MOCK_METHOD0(showPreviousWaypoints, void());
    MOCK_METHOD0(navigateToPreviousWaypoint, void());
    MOCK_METHOD1(showAlternativeRoutes, void(aace::navigation::Navigation::AlternateRouteType alternateRouteType));
    MOCK_METHOD1(controlDisplay, void(aace::navigation::Navigation::ControlDisplay controlDisplay));
    MOCK_METHOD1(startNavigation, void(const std::string& payload));
    MOCK_METHOD1(announceManeuver, void(const std::string& payload));
    MOCK_METHOD1(announceRoadRegulation, void(aace::navigation::Navigation::RoadRegulation roadRegulation));
};

class NavigationAssistanceCapabilityAgent : public ::testing::Test {
public:
    void SetUp() override {
        m_alexaMockFactory = alexa::AlexaTestHelper::createAlexaMockComponentFactory();
        m_mockExceptionSender = std::make_shared<
            testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockDirectiveHandlerResult = std::unique_ptr<
            testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveHandlerResult>>();
        m_testNavigationHandler = std::make_shared<testing::StrictMock<TestNavigationHandler>>();
        m_mockContextManager =
            std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockNavigationProviderName = "HERE";

        m_capAgent = aace::engine::navigation::navigationassistance::NavigationAssistanceCapabilityAgent::create(
            m_testNavigationHandler,
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock());
    }
    void TearDown() override {
        m_capAgent->shutdown();
    }

    /**
     * Utility function to generate direcive payloads
     */
    void wakeOnSetCompleted();
    const std::string generateAnnounceManeuverPayload(
        long latitudeInDegrees,
        long longitudeInDegrees,
        std::string singleLineDisplayAddress,
        std::string multiLineDiaplayAddress,
        std::string name,
        std::string maneuverType);
    const std::string generateAnnounceRoadRegulationPayload(std::string roadRegulationType);

    NavigationAssistanceCapabilityAgent() :
            m_wakeSetCompletedPromise{}, m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()} {
    }

    std::promise<void> m_wakeSetCompletedPromise;
    std::future<void> m_wakeSetCompletedFuture;

    std::shared_ptr<aace::engine::navigation::navigationassistance::NavigationAssistanceCapabilityAgent> m_capAgent;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>
        m_mockExceptionSender;
    std::unique_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveHandlerResult>>
        m_mockDirectiveHandlerResult;
    std::shared_ptr<testing::StrictMock<TestNavigationHandler>> m_testNavigationHandler;
    // a context manager
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>
        m_mockContextManager;
    // provider name
    std::string m_mockNavigationProviderName;
    std::shared_ptr<alexa::AlexaMockComponentFactory> m_alexaMockFactory;
};

const std::string NavigationAssistanceCapabilityAgent::generateAnnounceManeuverPayload(
    long latitudeInDegrees,
    long longitudeInDegrees,
    std::string singleLineDisplayAddress,
    std::string multiLineDisplayAddress,
    std::string name,
    std::string maneuverType) {
    std::string stringLatitudeInDegrees = std::to_string(latitudeInDegrees);
    std::string stringLongitudeInDegrees = std::to_string(longitudeInDegrees);
    const std::string payload =

        "{"
        "\"type\":\"" +
        maneuverType +
        "\","
        "\"targetLocation\":{"
        "\"address\": {"
        "\"addressLine1\": \"" +
        singleLineDisplayAddress +
        "\","
        "\"addressLine2\": \"" +
        multiLineDisplayAddress +
        "\","                                        //Address line 2
        "\"addressLine3\": \"DUMMY ADDRESS 3\","     //Address line 3
        "\"city\": \"DUMMY TEST\","                  //city
        "\"districtOrCounty\": \"DUMMY DISTRICT\","  //district or county
        "\"stateOrRegion\": \"DUMMY STATE\","        // state or region
        "\"countryCode\": \"DUMMY COUNTRY\","        //3 letter country code
        "\"postalCode\": \"DUMMY POSTAL CODE\""
        "},"
        "\"coordinate\":["
        "\"" +
        stringLatitudeInDegrees +
        "\","
        "\"" +
        stringLongitudeInDegrees +
        "\""
        "],"
        "\"name\": \"" +
        name +
        "\""
        "}"
        "}";
    return payload;
}

const std::string NavigationAssistanceCapabilityAgent::generateAnnounceRoadRegulationPayload(
    std::string roadRegulationType) {
    const std::string payload =

        "{"
        "\"type\": \"" +
        roadRegulationType +
        "\""
        "}";
    return payload;
}

void NavigationAssistanceCapabilityAgent::wakeOnSetCompleted() {
    m_wakeSetCompletedPromise.set_value();
}

TEST_F(NavigationAssistanceCapabilityAgent, create) {
    EXPECT_NE(nullptr, m_capAgent);
}

TEST_F(NavigationAssistanceCapabilityAgent, createWithNullExceptionSender) {
    std::shared_ptr<aace::engine::navigation::navigationassistance::NavigationAssistanceCapabilityAgent> capAgent;
    capAgent = aace::engine::navigation::navigationassistance::NavigationAssistanceCapabilityAgent::create(
        m_testNavigationHandler, nullptr, m_alexaMockFactory->getMessageSenderInterfaceMock(), m_mockContextManager);
    EXPECT_EQ(nullptr, capAgent);
}

TEST_F(NavigationAssistanceCapabilityAgent, createWithNullContextManager) {
    std::shared_ptr<aace::engine::navigation::navigationassistance::NavigationAssistanceCapabilityAgent> capAgent;
    capAgent = aace::engine::navigation::navigationassistance::NavigationAssistanceCapabilityAgent::create(
        m_testNavigationHandler, m_mockExceptionSender, m_alexaMockFactory->getMessageSenderInterfaceMock(), nullptr);
    EXPECT_EQ(nullptr, capAgent);
}

TEST_F(NavigationAssistanceCapabilityAgent, testUnknownDirective) {
    auto attachmentManager = std::make_shared<
        testing::StrictMock<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager>>();
    auto avsMessageHeader =
        std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(NAMESPACE, UNKNOWN_DIRECTIVE, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create("", avsMessageHeader, "", attachmentManager, "");

    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(NavigationAssistanceCapabilityAgent, testAnnounceManeuverDirective) {
    auto attachmentManager = std::make_shared<
        testing::StrictMock<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(
        ANNOUNCE_MANEUVER.nameSpace, ANNOUNCE_MANEUVER.name, MESSAGE_ID);
    std::string MockPayload = generateAnnounceManeuverPayload(
        0.0, 0.0, "fake single line address", "fake multi-line adress", "fake name", "TURN");
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create("", avsMessageHeader, MockPayload, attachmentManager, "");

    EXPECT_CALL(*m_testNavigationHandler, announceManeuver(MockPayload)).Times(testing::Exactly(1));

    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(NavigationAssistanceCapabilityAgent, testAnnounceRoadRegulationDirective) {
    auto attachmentManager = std::make_shared<
        testing::StrictMock<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(
        ANNOUNCE_ROAD_REGULATION.nameSpace, ANNOUNCE_ROAD_REGULATION.name, MESSAGE_ID);
    std::string MockPayload = generateAnnounceRoadRegulationPayload("SPEED_LIMIT");
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create("", avsMessageHeader, MockPayload, attachmentManager, "");

    EXPECT_CALL(
        *m_testNavigationHandler, announceRoadRegulation(aace::navigation::Navigation::RoadRegulation::SPEED_LIMIT))
        .Times(testing::Exactly(1));

    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_capAgent->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

}  // namespace unit
}  // namespace test
}  // namespace aace
