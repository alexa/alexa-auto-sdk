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


#include <AACE/Test/Alexa/AlexaTestHelper.h>
#include <AACE/Engine/Alexa/AuthProviderEngineImpl.h>

using namespace aace::test::alexa;

class AuthProviderEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();
    }

    void TearDown() override {
        m_alexaMockFactory->shutdown();
    }
    
protected:
    std::shared_ptr<aace::engine::alexa::AuthProviderEngineImpl> createAuthProviderEngineImpl() {
        return aace::engine::alexa::AuthProviderEngineImpl::create( m_alexaMockFactory->getAuthProviderMock() );
    }
    
protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;
};

TEST_F(AuthProviderEngineImplTest,create)
{
    auto authProviderEngineImpl = createAuthProviderEngineImpl();
    ASSERT_NE(authProviderEngineImpl,nullptr) << "AuthProviderEngineImpl pointer is null!";
    
    authProviderEngineImpl->shutdown();
}

TEST_F(AuthProviderEngineImplTest,createWithPlatformInterfaceAsNull)
{
    auto authProviderEngineImpl = aace::engine::alexa::AuthProviderEngineImpl::create( nullptr );
    ASSERT_EQ(authProviderEngineImpl,nullptr) << "AuthProviderEngineImpl pointer expected to be null!";
}

