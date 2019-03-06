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

#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include "AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h"
#include "AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h"

#include "AACE/Navigation/Navigation.h"
#include "AACE/Engine/Navigation/NavigationEngineImpl.h"


namespace aace {
namespace test {
namespace unit {

class MockCapabilitiesDelegate : public alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface {
public: 
    MOCK_METHOD1( registerCapability, bool(const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface>& capability) );
    MOCK_METHOD0( publishCapabilities, CapabilitiesPublishReturnCode() );
    MOCK_METHOD0( publishCapabilitiesAsyncWithRetries, void());
    MOCK_METHOD0( invalidateCapabilities, void() );
    MOCK_METHOD1( addCapabilitiesObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface> observer) );
    MOCK_METHOD1( removeCapabilitiesObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface> observer) );
};

class MockNavigationPlatformInterface : public aace::navigation::Navigation {
public:
    MOCK_METHOD1( setDestination, bool(const std::string& payload) );
    MOCK_METHOD0( cancelNavigation, bool() );
};


class NavigationEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        m_mockPlatformInterface = std::make_shared<testing::StrictMock<MockNavigationPlatformInterface>>();
        m_mockCapabilitiesDelegate = std::make_shared<testing::StrictMock<MockCapabilitiesDelegate>>();
        m_mockDirectiveSequencer = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>>();
        m_mockExceptionEncounteredSender = std::make_shared<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>>();

        EXPECT_CALL(*m_mockDirectiveSequencer, addDirectiveHandler(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockCapabilitiesDelegate, registerCapability(testing::_)).WillOnce(testing::Return(true));
        m_navigationEngineImpl = aace::engine::navigation::NavigationEngineImpl::create(
            m_mockPlatformInterface,
            m_mockDirectiveSequencer,
            m_mockCapabilitiesDelegate,
            m_mockExceptionEncounteredSender
        );
    }
    void TearDown() override{
        m_navigationEngineImpl->shutdown();
    }

    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> m_navigationEngineImpl;

    /// @c A test observer to catch directives
    std::shared_ptr<aace::navigation::Navigation> m_mockPlatformInterface;

    /// An exception sender used to send exception encountered events to AVS.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender> m_mockExceptionEncounteredSender;
    
    /// A directive sequencer.
    std::shared_ptr<testing::StrictMock<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>> m_mockDirectiveSequencer;

    // pass the interface
    std::shared_ptr<testing::StrictMock<MockCapabilitiesDelegate>> m_mockCapabilitiesDelegate;

};

/**
 * Test create() with nullptrs
 */
TEST_F( NavigationEngineImplTest, create ) {
    EXPECT_NE(nullptr, m_navigationEngineImpl);
}

TEST_F( NavigationEngineImplTest, createWithNullPlatform ) {
    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> testNavigationEngineImpl;
    testNavigationEngineImpl = engine::navigation::NavigationEngineImpl::create(
        nullptr,
        m_mockDirectiveSequencer,
        m_mockCapabilitiesDelegate,
        m_mockExceptionEncounteredSender);
    EXPECT_EQ(nullptr, testNavigationEngineImpl);
}

TEST_F( NavigationEngineImplTest, createWithNullDirectiveSequencer ) {
    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> testNavigationEngineImpl;
    testNavigationEngineImpl = engine::navigation::NavigationEngineImpl::create(
        m_mockPlatformInterface,
        nullptr,
        m_mockCapabilitiesDelegate,
        m_mockExceptionEncounteredSender);
    EXPECT_EQ(nullptr, testNavigationEngineImpl);
}

TEST_F( NavigationEngineImplTest, createWithNullCapabilitiesDelegate ) {
    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> testNavigationEngineImpl;
    testNavigationEngineImpl = engine::navigation::NavigationEngineImpl::create(
        m_mockPlatformInterface,
        m_mockDirectiveSequencer,
        nullptr,
        m_mockExceptionEncounteredSender);
    EXPECT_EQ(nullptr, testNavigationEngineImpl);
}

TEST_F( NavigationEngineImplTest, createWithNullExceptionEncounteredSender ) {
    std::shared_ptr<aace::engine::navigation::NavigationEngineImpl> testNavigationEngineImpl;
    testNavigationEngineImpl = engine::navigation::NavigationEngineImpl::create(
        m_mockPlatformInterface,
        m_mockDirectiveSequencer,
        m_mockCapabilitiesDelegate,
        nullptr);
    EXPECT_EQ(nullptr, testNavigationEngineImpl);
}

}  // namespace unit
}  // namespace test
}  // namespace aace

