/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Navigation/NavigationCapabilityAgent.h"

namespace aace {
namespace test {
namespace unit {
    
static std::chrono::milliseconds TIMEOUT(1000);

static const std::string NAMESPACE{"Navigation"};

static const std::string UNKNOWN_DIRECTIVE{"Unknown"};

static const alexaClientSDK::avsCommon::avs::NamespaceAndName SETDESTINATION{NAMESPACE, "SetDestination"};
static const alexaClientSDK::avsCommon::avs::NamespaceAndName CANCELNAVIGATION{NAMESPACE, "CancelNavigation"};
    
static const std::string MESSAGE_ID("messageId");

class TestNavigationObserver : public aace::engine::navigation::NavigationObserverInterface {
public:
    MOCK_METHOD1(setDestination, void( const std::string& payload ));
    MOCK_METHOD0(cancelNavigation, void());
};

class NavigationCapabilityAgentTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockExceptionSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockDirectiveHandlerResult = std::unique_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveHandlerResult>>();
        m_testNavigationObserver = std::make_shared<testing::StrictMock<TestNavigationObserver>>();
        m_capAgent = aace::engine::navigation::NavigationCapabilityAgent::create( m_mockExceptionSender );
        m_capAgent->addObserver( m_testNavigationObserver );
    }
    void TearDown() override {
        m_capAgent->shutdown();
    }

     /**
     * Utility function to generate direcive payloads
     */
    const std::string generatePayload(
        long latitudeInDegrees,
        long longitudeInDegrees,
        std::string singleLineDisplayAddress,
        std::string multiLineDiaplayAddress,
        std::string name);
    void wakeOnSetCompleted();

    NavigationCapabilityAgentTest() :
        m_wakeSetCompletedPromise{},
        m_wakeSetCompletedFuture{m_wakeSetCompletedPromise.get_future()} {
    }

    std::promise<void> m_wakeSetCompletedPromise;
    std::future<void> m_wakeSetCompletedFuture;

    std::shared_ptr<aace::engine::navigation::NavigationCapabilityAgent> m_capAgent;
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;
    std::unique_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveHandlerResult>> m_mockDirectiveHandlerResult;
    std::shared_ptr<testing::StrictMock<TestNavigationObserver>> m_testNavigationObserver;
};


const std::string NavigationCapabilityAgentTest::generatePayload(
    long latitudeInDegrees,
    long longitudeInDegrees,
    std::string singleLineDisplayAddress,
    std::string multiLineDiaplayAddress,
    std::string name) {

    std::string stringLatitudeInDegrees = std::to_string(latitudeInDegrees);
    std::string stringLongitudeInDegrees = std::to_string(longitudeInDegrees);

    const std::string payload =
        "{"
            "\"destination\": {"
                "\"coordinate\": {"
                    "\"latitudeInDegrees\": \"" + stringLatitudeInDegrees + "\","
                    "\"longitudeInDegrees\": \"" + stringLongitudeInDegrees + "\""
                "},"
                "\"singleLineDisplayAddress\": \"" + singleLineDisplayAddress + "\","
                "\"multiLineDisplayAddress\": \"" + multiLineDiaplayAddress + "\","
                "\"name\": \"" + name + "\""
            "}"
        "}";
    return payload;
}

void NavigationCapabilityAgentTest::wakeOnSetCompleted() {
    m_wakeSetCompletedPromise.set_value();
}

TEST_F(NavigationCapabilityAgentTest, create) {
    EXPECT_NE(nullptr, m_capAgent);
}

TEST_F(NavigationCapabilityAgentTest, createWithNullExceptionSender) {
    std::shared_ptr<aace::engine::navigation::NavigationCapabilityAgent> capAgent;
    capAgent = aace::engine::navigation::NavigationCapabilityAgent::create(nullptr);
    EXPECT_EQ(nullptr, capAgent);
}

TEST_F(NavigationCapabilityAgentTest, testUnknownDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>(NAMESPACE, UNKNOWN_DIRECTIVE, MESSAGE_ID);
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create("", avsMessageHeader, "", attachmentManager, "");
    
    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(testing::_,testing::_, testing::_)).Times(testing::Exactly(1));
    
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(NavigationCapabilityAgentTest, testSetDestinationDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>( SETDESTINATION.nameSpace, SETDESTINATION.name, MESSAGE_ID );
    std::string MockPayload = generatePayload(0.0, 0.0, "fake single line address", "fake multi-line adress", "fake name");
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create( "", avsMessageHeader, MockPayload, attachmentManager, "" );
    
    EXPECT_CALL(*m_testNavigationObserver, setDestination( MockPayload )).Times(testing::Exactly(1));
    
    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

TEST_F(NavigationCapabilityAgentTest, testCancelNavigationDirective) {
    auto attachmentManager = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::avs::attachment::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<alexaClientSDK::avsCommon::avs::AVSMessageHeader>( CANCELNAVIGATION.nameSpace, CANCELNAVIGATION.name, MESSAGE_ID );
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive =
        alexaClientSDK::avsCommon::avs::AVSDirective::create( "", avsMessageHeader, "", attachmentManager, "" );

    EXPECT_CALL(*m_testNavigationObserver, cancelNavigation()).Times(testing::Exactly(1));

    m_capAgent->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_wakeSetCompletedFuture.wait_for(TIMEOUT);
}

} //unit
} //test
} //aace
