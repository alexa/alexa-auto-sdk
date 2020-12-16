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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

#include <AACE/Engine/Authorization/AuthorizationEngineImpl.h>

using namespace aace::authorization;
using namespace aace::engine::authorization;
using namespace ::testing;

class MockAuthorizationPlatformInterface : public Authorization {
public:
    MOCK_METHOD2(eventReceived, void(const std::string& service, const std::string& request));
    MOCK_METHOD2(authorizationStateChanged, void(const std::string& service, AuthorizationState state));
    MOCK_METHOD3(
        authorizationError,
        void(const std::string& service, const std::string& error, const std::string& message));
    MOCK_METHOD2(getAuthorizationData, std::string(const std::string& service, const std::string& key));
    MOCK_METHOD3(
        setAuthorizationData,
        void(const std::string& service, const std::string& key, const std::string& data));
};

class MockAuthorizationProvider : public AuthorizationProvider {
public:
    MOCK_METHOD1(startAuthorization, bool(const std::string& data));
    MOCK_METHOD0(cancelAuthorization, bool());
    MOCK_METHOD0(logout, bool());
    MOCK_METHOD1(sendEvent, bool(const std::string& data));
};

class MockAuthorizationServiceInterface
        : public AuthorizationServiceInterface
        , public std::enable_shared_from_this<AuthorizationServiceInterface> {
public:
    MOCK_METHOD2(registerProvider, bool(std::shared_ptr<AuthorizationProvider> provider, const std::string& service));
    MOCK_METHOD1(getProvider, std::shared_ptr<AuthorizationProvider>(const std::string& service));
};

/// Test harness for @c AuthorizationEngineImpl class
class AuthorizationEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockAuthorizationPlatformInterface = std::make_shared<StrictMock<MockAuthorizationPlatformInterface>>();
        m_mockAuthorizationServiceInterface = std::make_shared<StrictMock<MockAuthorizationServiceInterface>>();
        m_mockAuthorizationProvider1 = std::make_shared<StrictMock<MockAuthorizationProvider>>();
        m_mockAuthorizationProvider2 = std::make_shared<StrictMock<MockAuthorizationProvider>>();
        m_authorizationEngineImpl = createAuthorizationEngineImpl();
    }

    void TearDown() override {
        if (m_mockAuthorizationPlatformInterface) {
            m_mockAuthorizationPlatformInterface.reset();
        }
        if (m_mockAuthorizationProvider1) {
            m_mockAuthorizationProvider1.reset();
        }
        if (m_mockAuthorizationProvider2) {
            m_mockAuthorizationProvider2.reset();
        }
        if (m_mockAuthorizationServiceInterface) {
            m_mockAuthorizationServiceInterface.reset();
        }
    }

protected:
    std::shared_ptr<AuthorizationEngineImpl> createAuthorizationEngineImpl() {
        auto authorizationEngineImpl =
            AuthorizationEngineImpl::create(m_mockAuthorizationPlatformInterface, m_mockAuthorizationServiceInterface);
        return authorizationEngineImpl;
    }

    /**
     * Settting up GMock expectations.
     */
    void setupGetProviderExpectations();

protected:
    /// Used to check callbacks to the platform interface.
    std::shared_ptr<MockAuthorizationPlatformInterface> m_mockAuthorizationPlatformInterface;

    /// Used to check callbacks to the platform interface.
    std::shared_ptr<MockAuthorizationServiceInterface> m_mockAuthorizationServiceInterface;

    /// Mocked authorization providers
    std::shared_ptr<MockAuthorizationProvider> m_mockAuthorizationProvider1;

    /// Mocked authorization providers
    std::shared_ptr<MockAuthorizationProvider> m_mockAuthorizationProvider2;

    /// Obejct used in the test
    std::shared_ptr<AuthorizationEngineImpl> m_authorizationEngineImpl;
};

void AuthorizationEngineImplTest::setupGetProviderExpectations() {
    EXPECT_CALL(*m_mockAuthorizationServiceInterface, getProvider(_))
        .WillRepeatedly(WithArg<0>(Invoke([this](const std::string& service) -> std::shared_ptr<AuthorizationProvider> {
            if (service == "service1") {
                return m_mockAuthorizationProvider1;
            }
            if (service == "service2") {
                return m_mockAuthorizationProvider2;
            }
            return nullptr;
        })));
}

TEST_F(AuthorizationEngineImplTest, test_createAuthorizationEngineImplWithInvalidParameters) {
    auto authorizationEngineImpl1 = AuthorizationEngineImpl::create(nullptr, m_mockAuthorizationServiceInterface);
    ASSERT_EQ(authorizationEngineImpl1, nullptr) << "AuthorizationEngineImpl pointer is not null!";

    auto authorizationEngineImpl2 =
        aace::engine::authorization::AuthorizationEngineImpl::create(m_mockAuthorizationPlatformInterface, nullptr);
    ASSERT_EQ(authorizationEngineImpl2, nullptr) << "AuthorizationEngineImpl pointer is not null!";
}

TEST_F(AuthorizationEngineImplTest, test_verifyHappyCase) {
    ASSERT_NE(m_authorizationEngineImpl, nullptr) << "AuthorizationEngineImpl pointer expected to be not null";

    setupGetProviderExpectations();

    EXPECT_CALL(*m_mockAuthorizationProvider1, startAuthorization("{\"TestData\"}"));
    EXPECT_CALL(*m_mockAuthorizationProvider2, startAuthorization(_)).Times(0);
    m_authorizationEngineImpl->onStartAuthorization("service1", "{\"TestData\"}");

    EXPECT_CALL(*m_mockAuthorizationProvider1, cancelAuthorization());
    EXPECT_CALL(*m_mockAuthorizationProvider2, cancelAuthorization()).Times(0);
    m_authorizationEngineImpl->onCancelAuthorization("service1");

    EXPECT_CALL(*m_mockAuthorizationProvider1, logout());
    EXPECT_CALL(*m_mockAuthorizationProvider2, logout()).Times(0);
    m_authorizationEngineImpl->onLogout("service1");

    EXPECT_CALL(*m_mockAuthorizationProvider1, sendEvent("{\"TestData\"}"));
    EXPECT_CALL(*m_mockAuthorizationProvider2, sendEvent(_)).Times(0);
    m_authorizationEngineImpl->onSendEvent("service1", "{\"TestData\"}");
}

TEST_F(AuthorizationEngineImplTest, test_verifyInvalidServiceCalls) {
    setupGetProviderExpectations();

    EXPECT_CALL(*m_mockAuthorizationProvider1, startAuthorization(_)).Times(0);
    EXPECT_CALL(*m_mockAuthorizationProvider2, startAuthorization(_)).Times(0);
    m_authorizationEngineImpl->onStartAuthorization("notRegistredService", "{\"TestData\"}");

    EXPECT_CALL(*m_mockAuthorizationProvider1, cancelAuthorization()).Times(0);
    EXPECT_CALL(*m_mockAuthorizationProvider2, cancelAuthorization()).Times(0);
    m_authorizationEngineImpl->onCancelAuthorization("notRegistredService");

    EXPECT_CALL(*m_mockAuthorizationProvider1, logout()).Times(0);
    EXPECT_CALL(*m_mockAuthorizationProvider2, logout()).Times(0);
    m_authorizationEngineImpl->onLogout("notRegistredService");

    EXPECT_CALL(*m_mockAuthorizationProvider1, sendEvent(_)).Times(0);
    EXPECT_CALL(*m_mockAuthorizationProvider2, sendEvent(_)).Times(0);
    m_authorizationEngineImpl->onSendEvent("notRegistredService", "{\"TestData\"}");
}

TEST_F(AuthorizationEngineImplTest, test_verifySequenceOfCalling) {
    setupGetProviderExpectations();

    {
        InSequence s;
        EXPECT_CALL(*m_mockAuthorizationProvider1, startAuthorization(_)).Times(1);
        EXPECT_CALL(*m_mockAuthorizationProvider2, startAuthorization(_)).Times(1);
    }
    m_authorizationEngineImpl->onStartAuthorization("service1", "{\"TestData\"}");
    m_authorizationEngineImpl->onStartAuthorization("service2", "{\"TestData\"}");

    {
        InSequence s;
        EXPECT_CALL(*m_mockAuthorizationProvider2, cancelAuthorization()).Times(1);
        EXPECT_CALL(*m_mockAuthorizationProvider1, cancelAuthorization()).Times(1);
    }
    m_authorizationEngineImpl->onCancelAuthorization("service2");
    m_authorizationEngineImpl->onCancelAuthorization("service1");

    {
        InSequence s;
        EXPECT_CALL(*m_mockAuthorizationProvider1, logout()).Times(1);
        EXPECT_CALL(*m_mockAuthorizationProvider2, logout()).Times(1);
    }
    m_authorizationEngineImpl->onLogout("service1");
    m_authorizationEngineImpl->onLogout("service2");

    {
        InSequence s;
        EXPECT_CALL(*m_mockAuthorizationProvider2, sendEvent("{\"MockData2\"}")).Times(1);
        EXPECT_CALL(*m_mockAuthorizationProvider1, sendEvent("{\"MockData1\"}")).Times(1);
    }
    m_authorizationEngineImpl->onSendEvent("service2", "{\"MockData2\"}");
    m_authorizationEngineImpl->onSendEvent("service1", "{\"MockData1\"}");
}

TEST_F(AuthorizationEngineImplTest, test_verifyAuthorizationProviderListenerCalls) {
    EXPECT_CALL(
        *m_mockAuthorizationPlatformInterface,
        authorizationStateChanged("service1", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING));
    m_authorizationEngineImpl->onAuthorizationStateChanged(
        "service1", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING);

    EXPECT_CALL(*m_mockAuthorizationPlatformInterface, authorizationError("service2", "Mock_Error", "Mock_Message"));
    m_authorizationEngineImpl->onAuthorizationError("service2", "Mock_Error", "Mock_Message");

    EXPECT_CALL(*m_mockAuthorizationPlatformInterface, eventReceived("service1", "{\"Mock-Request\"}"));
    m_authorizationEngineImpl->onEventReceived("service1", "{\"Mock-Request\"}");

    EXPECT_CALL(*m_mockAuthorizationPlatformInterface, getAuthorizationData("service2", "Mock-Key"))
        .WillOnce(Return("MockGetData"));
    auto getAurhtoizationDataResult = m_authorizationEngineImpl->onGetAuthorizationData("service2", "Mock-Key");
    EXPECT_EQ(getAurhtoizationDataResult, "MockGetData");

    EXPECT_CALL(*m_mockAuthorizationPlatformInterface, setAuthorizationData("service2", "Mock-Key", "{\"Mock-Data\"}"));
    m_authorizationEngineImpl->onSetAuthorizationData("service2", "Mock-Key", "{\"Mock-Data\"}");
}
