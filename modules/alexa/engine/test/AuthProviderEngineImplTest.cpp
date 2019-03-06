/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Alexa/AuthProviderEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon;

class MockAuthProvider : public aace::alexa::AuthProvider {
public:
  MOCK_METHOD0(getAuthToken,
      std::string());
  MOCK_METHOD0(getAuthState,
      AuthState());
};

class AuthProviderEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }
        m_mockAuthProviderPlatformInterface = std::make_shared<aace::test::unit::MockAuthProvider>();

        m_authProviderEngineImpl = aace::engine::alexa::AuthProviderEngineImpl::create(
            m_mockAuthProviderPlatformInterface);
        
        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    void TearDown() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }

        if( m_authProviderEngineImpl != nullptr ) {
            m_authProviderEngineImpl->shutdown();
        }

        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::AuthProviderEngineImpl> m_authProviderEngineImpl;
    std::shared_ptr<aace::test::unit::MockAuthProvider> m_mockAuthProviderPlatformInterface;
};

/**
 * Test create() with valid parameters
 */
TEST_F( AuthProviderEngineImplTest, create ) {
    ASSERT_NE(nullptr, m_authProviderEngineImpl) << "AuthProviderEngineImpl pointer is null";
}


/**
 * Test create() with valid parameters
 */
TEST_F(AuthProviderEngineImplTest, createAuthProviderWithNullAndCallgetAuthState) {
    std::shared_ptr<aace::engine::alexa::AuthProviderEngineImpl> authProviderEngineImplTemp;

    authProviderEngineImplTemp = aace::engine::alexa::AuthProviderEngineImpl::create(nullptr);
    EXPECT_EQ(MockAuthProvider::AuthState::UNINITIALIZED, authProviderEngineImplTemp->getAuthState());
}

TEST_F(AuthProviderEngineImplTest, createAuthProviderWithNullAndCallgetAuthToken) {
    std::shared_ptr<aace::engine::alexa::AuthProviderEngineImpl> authProviderEngineImplTemp;

    authProviderEngineImplTemp = aace::engine::alexa::AuthProviderEngineImpl::create(nullptr);
    EXPECT_EQ(std::string(), authProviderEngineImplTemp->getAuthToken());
}

TEST_F(AuthProviderEngineImplTest, checkAuthStateUNINITIALIZED) {
    EXPECT_CALL(*m_mockAuthProviderPlatformInterface, 
        getAuthState()).WillOnce(testing::Return(MockAuthProvider::AuthState::UNINITIALIZED));
    EXPECT_EQ(MockAuthProvider::AuthState::UNINITIALIZED, m_authProviderEngineImpl->getAuthState());
}

TEST_F(AuthProviderEngineImplTest, checkAuthStateREFRESHED) {
    EXPECT_CALL(*m_mockAuthProviderPlatformInterface, 
        getAuthState()).WillOnce(testing::Return(MockAuthProvider::AuthState::REFRESHED));
    EXPECT_EQ(MockAuthProvider::AuthState::REFRESHED, m_authProviderEngineImpl->getAuthState());
}

TEST_F(AuthProviderEngineImplTest, checkAuthStateUNRECOVERABLE_ERROR) {
    EXPECT_CALL(*m_mockAuthProviderPlatformInterface, 
        getAuthState()).WillOnce(testing::Return(MockAuthProvider::AuthState::UNRECOVERABLE_ERROR));
    EXPECT_EQ(MockAuthProvider::AuthState::UNRECOVERABLE_ERROR, m_authProviderEngineImpl->getAuthState());
}

TEST_F(AuthProviderEngineImplTest, checkAuthToken) {
    std::string tokenString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890=~`!@#$%^&*()_+-=<>/?";
    EXPECT_CALL(*m_mockAuthProviderPlatformInterface, 
        getAuthToken()).WillOnce(testing::Return(tokenString));
    EXPECT_EQ(0, tokenString.compare(m_authProviderEngineImpl->getAuthToken()));
}

}  // namespace unit
}  // namespace test
}  // namespace aace

