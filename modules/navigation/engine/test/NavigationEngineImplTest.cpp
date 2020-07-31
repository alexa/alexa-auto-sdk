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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include "AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h"
#include "AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h"
#include "AVSCommon/SDKInterfaces/test/MockContextManager.h"

#include "AACE/Test/Alexa/AlexaTestHelper.h"
#include "AACE/Navigation/Navigation.h"
#include "AACE/Engine/Navigation/NavigationEngineImpl.h"

namespace aace {
namespace test {
namespace unit {

class MockNavigationPlatformInterface : public aace::navigation::Navigation {
public:
    MOCK_METHOD0(cancelNavigation, bool());
    MOCK_METHOD0(getNavigationState, std::string());
    MOCK_METHOD0(showPreviousWaypoints, void());
    MOCK_METHOD0(navigateToPreviousWaypoint, void());
    MOCK_METHOD1(showAlternativeRoutes, void(AlternateRouteType alternateRouteType));
    MOCK_METHOD1(controlDisplay, void(ControlDisplay controlDisplay));
    MOCK_METHOD1(startNavigation, void(const std::string& payload));
    MOCK_METHOD1(announceManeuver, void(const std::string& payload));
    MOCK_METHOD1(announceRoadRegulation, void(RoadRegulation roadRegulation));
};

class NavigationEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockPlatformInterface = std::make_shared<testing::StrictMock<MockNavigationPlatformInterface>>();
        m_mockDirectiveSequencer = std::make_shared<
            testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        m_mockExceptionEncounteredSender = std::make_shared<
            testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();
        m_mockContextManager =
            std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>();
        m_mockNavigationProviderName = "HERE";
        m_alexaMockFactory = alexa::AlexaTestHelper::createAlexaMockComponentFactory();
        m_navigationEngineImpl = aace::engine::navigation::NavigationEngineImpl::create(
            m_mockPlatformInterface,
            m_alexaMockFactory->getEndpointBuilderMock(),
            m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),
            m_alexaMockFactory->getMessageSenderInterfaceMock(),
            m_alexaMockFactory->getContextManagerInterfaceMock(),
            m_mockNavigationProviderName);
    }
    void TearDown() override {
        m_navigationEngineImpl->shutdown();
        m_alexaMockFactory->shutdown();
    }

    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> m_navigationEngineImpl;

    /// @c A test observer to catch directives
    std::shared_ptr<aace::navigation::Navigation> m_mockPlatformInterface;

    /// An exception sender used to send exception encountered events to AVS.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>
        m_mockExceptionEncounteredSender;

    /// A directive sequencer.
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>
        m_mockDirectiveSequencer;

    // a context manager
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>>
        m_mockContextManager;

    // provider name
    std::string m_mockNavigationProviderName;

    // test helper for providing common mocks
    std::shared_ptr<alexa::AlexaMockComponentFactory> m_alexaMockFactory;
};

/**
 * Test create() with nullptrs
 */
TEST_F(NavigationEngineImplTest, create) {
    EXPECT_NE(nullptr, m_navigationEngineImpl);
}

TEST_F(NavigationEngineImplTest, createWithNullPlatform) {
    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> testNavigationEngineImpl;
    testNavigationEngineImpl = engine::navigation::NavigationEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_mockExceptionEncounteredSender,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_mockContextManager,
        m_mockNavigationProviderName);
    EXPECT_EQ(nullptr, testNavigationEngineImpl);
}

TEST_F(NavigationEngineImplTest, createWithNullEndpointBuilder) {
    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> testNavigationEngineImpl;
    testNavigationEngineImpl = engine::navigation::NavigationEngineImpl::create(
        m_mockPlatformInterface,
        nullptr,
        m_mockExceptionEncounteredSender,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_mockContextManager,
        m_mockNavigationProviderName);
    EXPECT_EQ(nullptr, testNavigationEngineImpl);
}

TEST_F(NavigationEngineImplTest, createWithNullExceptionEncounteredSender) {
    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> testNavigationEngineImpl;
    testNavigationEngineImpl = engine::navigation::NavigationEngineImpl::create(
        m_mockPlatformInterface,
        m_alexaMockFactory->getEndpointBuilderMock(),
        nullptr,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        m_mockContextManager,
        m_mockNavigationProviderName);
    EXPECT_EQ(nullptr, testNavigationEngineImpl);
}

TEST_F(NavigationEngineImplTest, createWithNullContextManager) {
    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> testNavigationEngineImpl;
    testNavigationEngineImpl = engine::navigation::NavigationEngineImpl::create(
        m_mockPlatformInterface,
        m_alexaMockFactory->getEndpointBuilderMock(),
        m_mockExceptionEncounteredSender,
        m_alexaMockFactory->getMessageSenderInterfaceMock(),
        nullptr,
        m_mockNavigationProviderName);
    EXPECT_EQ(nullptr, testNavigationEngineImpl);
}

}  // namespace unit
}  // namespace test
}  // namespace aace
