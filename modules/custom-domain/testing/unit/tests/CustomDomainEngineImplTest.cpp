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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <AACE/Test/Unit/Alexa/AlexaTestHelper.h>
#include <AACE/Engine/Alexa/AlexaEngineService.h>
#include <AACE/Engine/CustomDomain/CustomDomainEngineImpl.h>

namespace aace {
namespace test {
namespace unit {

class MockCustomDomainPlatformInterface : public aace::customDomain::CustomDomain {
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

class DummyEngineContext : public aace::engine::core::EngineContext {
public:
    std::shared_ptr<aace::engine::core::EngineServiceContext> getService(const std::string& type) {
        return nullptr;
    }

    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        return true;
    }

    std::shared_ptr<aace::engine::alexa::AlexaComponentInterface> getServiceInterface(const std::string& serviceType) {
        return nullptr;
    }
};

class CustomDomainEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockPlatformInterface = std::make_shared<testing::StrictMock<MockCustomDomainPlatformInterface>>();
        m_dummyEngineContext = std::make_shared<DummyEngineContext>();
        m_customDomainEngineImpl = aace::engine::customDomain::CustomDomainEngineImpl::create(
            m_mockPlatformInterface, m_dummyEngineContext, "");
    }

    void TearDown() override {
        if (m_customDomainEngineImpl != nullptr) m_customDomainEngineImpl->shutdown();
    }

    std::shared_ptr<aace::engine::customDomain::CustomDomainEngineImpl> m_customDomainEngineImpl;
    std::shared_ptr<aace::customDomain::CustomDomain> m_mockPlatformInterface;
    std::shared_ptr<aace::engine::core::EngineContext> m_dummyEngineContext;
};

TEST_F(CustomDomainEngineImplTest, createWithNullAlexaComponentInterface) {
    EXPECT_EQ(nullptr, m_customDomainEngineImpl);
}

TEST_F(CustomDomainEngineImplTest, createWithNullEngineContext) {
    std::shared_ptr<aace::engine::customDomain::CustomDomainEngineImpl> testCustomDomainEngineImpl;
    testCustomDomainEngineImpl =
        engine::customDomain::CustomDomainEngineImpl::create(m_mockPlatformInterface, nullptr, "");
    EXPECT_EQ(nullptr, m_customDomainEngineImpl);
}

TEST_F(CustomDomainEngineImplTest, createWithNullPlatform) {
    std::shared_ptr<aace::engine::customDomain::CustomDomainEngineImpl> testCustomDomainEngineImpl;
    testCustomDomainEngineImpl =
        engine::customDomain::CustomDomainEngineImpl::create(nullptr, m_dummyEngineContext, "");
    EXPECT_EQ(nullptr, testCustomDomainEngineImpl);
}

}  // namespace unit
}  // namespace test
}  // namespace aace
