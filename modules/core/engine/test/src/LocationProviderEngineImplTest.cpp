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

#include <AACE/Engine/Location/LocationProviderEngineImpl.h>

using namespace aace::location;
using namespace aace::engine::location;
using namespace ::testing;
using LocationServiceAccess = aace::location::LocationProviderEngineInterface::LocationServiceAccess;

/**
 * Mock Location Provider platform interface.
 */
class MockLocationProviderPlatformInterface : public LocationProvider {
public:
    MOCK_METHOD0(getLocation, Location());
};

/**
 * Mock Location Service Observer interface.
 */
class MockLocationServiceObserverInterface : public LocationServiceObserverInterface {
public:
    MOCK_METHOD1(onLocationServiceAccessChanged, void(LocationServiceAccess access));
};

/**
 * Unit test for @c LocationProviderEngineImpl class.
 */
class LocationProviderEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockLocationProviderPlatformInterface = std::make_shared<StrictMock<MockLocationProviderPlatformInterface>>();
        m_mockLocationServiceObserverInterface = std::make_shared<StrictMock<MockLocationServiceObserverInterface>>();
        m_locationProviderEngineImpl = createLocationProviderEngineImpl();
    }

    void TearDown() override {
        if (m_mockLocationProviderPlatformInterface) {
            m_mockLocationProviderPlatformInterface.reset();
        }

        if (m_mockLocationServiceObserverInterface) {
            m_mockLocationServiceObserverInterface.reset();
        }

        if (m_locationProviderEngineImpl) {
            m_locationProviderEngineImpl->shutdown();
            m_locationProviderEngineImpl.reset();
        }
    }

protected:
    std::shared_ptr<LocationProviderEngineImpl> createLocationProviderEngineImpl() {
        auto locationProviderEngineImpl = LocationProviderEngineImpl::create(m_mockLocationProviderPlatformInterface);
        return locationProviderEngineImpl;
    }

protected:
    /// Used to check callbacks to the platform interface.
    std::shared_ptr<MockLocationProviderPlatformInterface> m_mockLocationProviderPlatformInterface;

    /// Used to check callbacks to the location service observer interface.
    std::shared_ptr<MockLocationServiceObserverInterface> m_mockLocationServiceObserverInterface;

    /// Object used in the test
    std::shared_ptr<LocationProviderEngineImpl> m_locationProviderEngineImpl;
};

/**
 * @test create
 */
TEST_F(LocationProviderEngineImplTest, test_create) {
    auto locationProviderEngineImpl = createLocationProviderEngineImpl();
    ASSERT_NE(nullptr, locationProviderEngineImpl);
}

/**
 * @test createWithNullLocationProviderPlatform
 */
TEST_F(LocationProviderEngineImplTest, test_createWithNullLocationProviderPlatform) {
    auto locationProviderEngineImpl = LocationProviderEngineImpl::create(nullptr);
    ASSERT_EQ(nullptr, locationProviderEngineImpl);
}

/**
 * @test getLocationWithValidLocationProviderPlatform
 */
TEST_F(LocationProviderEngineImplTest, test_getLocationWithValidLocationProviderPlatform) {
    ASSERT_NE(nullptr, m_locationProviderEngineImpl);

    EXPECT_CALL(*m_mockLocationProviderPlatformInterface, getLocation()).Times(1).WillOnce(testing::Return(Location()));
    m_locationProviderEngineImpl->getLocation();
}

/**
 * @test onLocationServiceAccessChangedWithObserverAdded
 */
TEST_F(LocationProviderEngineImplTest, test_onLocationServiceAccessChangedWithObserverAdded) {
    ASSERT_NE(nullptr, m_locationProviderEngineImpl);

    LocationServiceAccess access;
    EXPECT_CALL(*m_mockLocationServiceObserverInterface, onLocationServiceAccessChanged(_))
        .Times(Exactly(1))
        .WillOnce(SaveArg<0>(&access));

    m_locationProviderEngineImpl->addObserver(m_mockLocationServiceObserverInterface);
    m_locationProviderEngineImpl->onLocationServiceAccessChanged(LocationServiceAccess::DISABLED);
    ASSERT_EQ(LocationServiceAccess::DISABLED, access);
}

/**
 * @test onLocationServiceAccessChangedWithObserverRemoved
 */
TEST_F(LocationProviderEngineImplTest, test_onLocationServiceAccessChangedWithObserverRemoved) {
    ASSERT_NE(nullptr, m_locationProviderEngineImpl);

    LocationServiceAccess access;
    EXPECT_CALL(*m_mockLocationServiceObserverInterface, onLocationServiceAccessChanged(_))
        .Times(Exactly(1))
        .WillOnce(SaveArg<0>(&access));
    m_locationProviderEngineImpl->addObserver(m_mockLocationServiceObserverInterface);
    m_locationProviderEngineImpl->onLocationServiceAccessChanged(LocationServiceAccess::ENABLED);
    ASSERT_EQ(LocationServiceAccess::ENABLED, access);

    m_locationProviderEngineImpl->removeObserver(m_mockLocationServiceObserverInterface);
    m_locationProviderEngineImpl->onLocationServiceAccessChanged(LocationServiceAccess::DISABLED);
    ASSERT_NE(LocationServiceAccess::DISABLED, access);
}

/**
 * @test getLocationCalledAfterShutdown
 */
TEST_F(LocationProviderEngineImplTest, test_getLocationCalledAfterShutdown) {
    ASSERT_NE(nullptr, m_locationProviderEngineImpl);

    EXPECT_CALL(*m_mockLocationProviderPlatformInterface, getLocation()).Times(Exactly(0));
    m_locationProviderEngineImpl->shutdown();
    m_locationProviderEngineImpl->getLocation();
}

/**
 * @test onLocationServiceAccessChangedCalledAfterShutdown
 */
TEST_F(LocationProviderEngineImplTest, test_onLocationServiceAccessChangedCalledAfterShutdown) {
    ASSERT_NE(nullptr, m_locationProviderEngineImpl);

    EXPECT_CALL(*m_mockLocationServiceObserverInterface, onLocationServiceAccessChanged(_)).Times(Exactly(0));
    m_locationProviderEngineImpl->addObserver(m_mockLocationServiceObserverInterface);
    m_locationProviderEngineImpl->shutdown();
    m_locationProviderEngineImpl->onLocationServiceAccessChanged(LocationServiceAccess::ENABLED);
}