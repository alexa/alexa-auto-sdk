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

#include <AVSCommon/Utils/WaitEvent.h>

#include <AACE/Test/Alexa/MockAuthorizationManager.h>
#include <AACE/Test/Authorization/MockAuthorizationProviderListener.h>

#include <AACE/Engine/CBL/CBLAuthorizationProvider.h>
#include <AACE/Engine/CBL/CBLConfigurationInterface.h>
#include <AACE/Engine/Alexa/AuthorizationManagerInterface.h>

namespace aace {
namespace test {
namespace unit {

/// Plenty of timeout to wait for async task to run
static std::chrono::seconds TIMEOUT(2);

using namespace ::testing;
using namespace aace::engine::alexa;
using namespace aace::engine::authorization;
using namespace aace::engine::cbl;
using namespace aace::test::alexa;
using namespace aace::test::core;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;

class MockCBLConfiguration : public CBLConfigurationInterface {
public:
    MOCK_CONST_METHOD0(getClientId, std::string());
    MOCK_CONST_METHOD0(getProductId, std::string());
    MOCK_CONST_METHOD0(getDeviceSerialNumber, std::string());
    MOCK_CONST_METHOD0(getRequestTimeout, std::chrono::seconds());
    MOCK_CONST_METHOD0(getCodePairRequestTimeout, std::chrono::seconds());
    MOCK_CONST_METHOD0(getAccessTokenRefreshHeadStart, std::chrono::seconds());
    MOCK_CONST_METHOD0(getRequestCodePairUrl, std::string());
    MOCK_CONST_METHOD0(getRequestTokenUrl, std::string());
    MOCK_CONST_METHOD0(getRefreshTokenUrl, std::string());
    MOCK_CONST_METHOD0(getScopeData, std::string());
    MOCK_CONST_METHOD0(getDefaultLocale, std::string());
};

/**
 * 
 * GTest class test @CBLAuthorizationProvider. 
 * 
 * @note This class tests only the surface level API and not the deep level tests
 * like, for example, getting the CBL token and authorizing it  because these
 * requires clientId and productId which is not available at the unit test level.
 */

class CBLAuthorizationProviderTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockAuthorizationManager = std::make_shared<StrictMock<MockAuthorizationManager>>();
        m_mockAuthorizationProviderListener = std::make_shared<StrictMock<MockAuthorizationProviderListener>>();
        m_configuration = std::make_shared<NiceMock<MockCBLConfiguration>>();
    }

    void TearDown() override {
        if (m_configuration) {
            m_configuration.reset();
        }
        if (m_mockAuthorizationManager) {
            m_mockAuthorizationManager.reset();
        }
        if (m_mockAuthorizationProviderListener) {
            m_mockAuthorizationProviderListener.reset();
        }
    }

protected:
    std::shared_ptr<aace::engine::cbl::CBLAuthorizationProvider> createCBLAuthorizationProvider() {
        auto cblAuthorizationProvider =
            aace::engine::cbl::CBLAuthorizationProvider::create("TEST_ME", m_mockAuthorizationManager, m_configuration);
        cblAuthorizationProvider->setListener(m_mockAuthorizationProviderListener);
        return cblAuthorizationProvider;
    }

protected:
    /// The @c AuthorizationManager used by the mocked classes.
    std::shared_ptr<MockAuthorizationManager> m_mockAuthorizationManager;

    /// The mocked @c AuthorizationProviderListenerInterface.
    std::shared_ptr<MockAuthorizationProviderListener> m_mockAuthorizationProviderListener;

    /// The mocked @c CBLConfigurationInterface.
    std::shared_ptr<MockCBLConfiguration> m_configuration;
};

TEST_F(CBLAuthorizationProviderTest, createWithNullParameters) {
    auto cblAuthorizationProvider =
        aace::engine::cbl::CBLAuthorizationProvider::create("", m_mockAuthorizationManager, m_configuration);
    ASSERT_EQ(cblAuthorizationProvider, nullptr) << "CBLAuthorizationProvider pointer expected to be null!";

    auto cblAuthorizationProvider1 =
        aace::engine::cbl::CBLAuthorizationProvider::create("testMe", nullptr, m_configuration);
    ASSERT_EQ(cblAuthorizationProvider1, nullptr) << "CBLAuthorizationProvider pointer expected to be null!";

    auto cblAuthorizationProvider2 =
        aace::engine::cbl::CBLAuthorizationProvider::create("testMe", m_mockAuthorizationManager, nullptr);
    ASSERT_EQ(cblAuthorizationProvider2, nullptr) << "CBLAuthorizationProvider pointer expected to be null!";
}

TEST_F(CBLAuthorizationProviderTest, simpleHappyUsecase) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;

    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto cblAuthorizationProvider = createCBLAuthorizationProvider();
    ASSERT_NE(cblAuthorizationProvider, nullptr) << "CBLAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onAuthorizationError("TEST_ME", "TIMEOUT", _)).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() -> void { waitEvent.wakeUp(); }));

    EXPECT_TRUE(cblAuthorizationProvider->startAuthorization(""));
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));

    cblAuthorizationProvider->shutdown();
}

TEST_F(CBLAuthorizationProviderTest, subsequentStartAndStop) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent1, waitEvent2;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto cblAuthorizationProvider = createCBLAuthorizationProvider();
    ASSERT_NE(cblAuthorizationProvider, nullptr) << "CBLAuthorizationProvider pointer expected to be not null!";

    // Start and wait for callbacks before calling cancelAuthorization
    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onAuthorizationError("TEST_ME", "TIMEOUT", _)).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent1]() -> void { waitEvent1.wakeUp(); }));

    EXPECT_TRUE(cblAuthorizationProvider->startAuthorization(""));
    EXPECT_TRUE(waitEvent1.wait(TIMEOUT));

    EXPECT_FALSE(cblAuthorizationProvider->cancelAuthorization());

    // Start the authorization again and cancel immediately.
    waitEvent1.reset();
    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(AtMost(1));
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onAuthorizationError("TEST_ME", "TIMEOUT", _)).Times(AtMost(1));
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent1]() -> void { waitEvent1.wakeUp(); }));

    EXPECT_TRUE(cblAuthorizationProvider->startAuthorization(""));
    EXPECT_TRUE(waitEvent1.wait(TIMEOUT));

    EXPECT_FALSE(cblAuthorizationProvider->cancelAuthorization());

    cblAuthorizationProvider->shutdown();
}

TEST_F(CBLAuthorizationProviderTest, subsequentStartAndLogout) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;
    bool receivedUnauthorized = false;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto cblAuthorizationProvider = createCBLAuthorizationProvider();
    ASSERT_NE(cblAuthorizationProvider, nullptr) << "CBLAuthorizationProvider pointer expected to be not null!";

    // Start and wait for callbacks before calling cancelAuthorization
    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .WillOnce(testing::InvokeWithoutArgs([cblAuthorizationProvider]() -> void {
            // Logout not allowed when authorization is in progress.
            EXPECT_FALSE(cblAuthorizationProvider->logout());
        }));
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onAuthorizationError("TEST_ME", "TIMEOUT", _)).Times(AtMost(1));
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() -> void { waitEvent.wakeUp(); }));

    EXPECT_TRUE(cblAuthorizationProvider->startAuthorization(""));
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));

    EXPECT_CALL(*m_mockAuthorizationManager, logout("TEST_ME"))
        .Times(AtMost(1))
        .WillOnce(testing::InvokeWithoutArgs([]() -> bool { return true; }));

    // Logout is allowed as we are already UNAUTHORIZED
    EXPECT_TRUE(cblAuthorizationProvider->logout());

    cblAuthorizationProvider->shutdown();
}

TEST_F(CBLAuthorizationProviderTest, invalidJsonPassedToStartAuthorization) {
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto cblAuthorizationProvider = createCBLAuthorizationProvider();
    ASSERT_NE(cblAuthorizationProvider, nullptr) << "CBLAuthorizationProvider pointer expected to be not null!";

    EXPECT_FALSE(cblAuthorizationProvider->startAuthorization("ABC"));

    // Refresh token not as a string
    EXPECT_FALSE(cblAuthorizationProvider->startAuthorization(R"({"refreshToken":123})"));

    cblAuthorizationProvider->shutdown();
}

TEST_F(CBLAuthorizationProviderTest, emptyJsonPassedToStartAuthorization) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;

    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto cblAuthorizationProvider = createCBLAuthorizationProvider();
    ASSERT_NE(cblAuthorizationProvider, nullptr) << "CBLAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onAuthorizationError("TEST_ME", "TIMEOUT", _)).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() -> void { waitEvent.wakeUp(); }));

    EXPECT_TRUE(cblAuthorizationProvider->startAuthorization("{}"));
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));

    cblAuthorizationProvider->shutdown();
}

TEST_F(CBLAuthorizationProviderTest, validJsonButEmtpyRefreshTokenToStartAuthorization) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;

    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto cblAuthorizationProvider = createCBLAuthorizationProvider();
    ASSERT_NE(cblAuthorizationProvider, nullptr) << "CBLAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onAuthorizationError("TEST_ME", "TIMEOUT", _)).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() -> void { waitEvent.wakeUp(); }));

    EXPECT_TRUE(cblAuthorizationProvider->startAuthorization(R"({"refreshToken":""})"));
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));

    cblAuthorizationProvider->shutdown();
}

TEST_F(CBLAuthorizationProviderTest, testPossibleFailurePaths) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto cblAuthorizationProvider = createCBLAuthorizationProvider();
    ASSERT_NE(cblAuthorizationProvider, nullptr) << "CBLAuthorizationProvider pointer expected to be not null!";

    // Cancel called before startAuthorization
    EXPECT_FALSE(cblAuthorizationProvider->cancelAuthorization());

    // AuthorizationManager returning StartAuthorizationResult::FAILED
    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs(
            [&waitEvent, cblAuthorizationProvider]() -> AuthorizationManagerInterface::StartAuthorizationResult {
                waitEvent.wakeUp();
                return AuthorizationManagerInterface::StartAuthorizationResult::FAILED;
            }));
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onAuthorizationError("TEST_ME", "START_AUTHORIZATION_FAILED", _))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent, cblAuthorizationProvider]() -> void { waitEvent.wakeUp(); }));
    EXPECT_TRUE(cblAuthorizationProvider->startAuthorization(""));

    // Calling logout when current state is not AuthorizationState::AUTHORIZED
    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([]() -> AuthorizationManagerInterface::StartAuthorizationResult {
            return AuthorizationManagerInterface::StartAuthorizationResult::REAUTHORIZE;
        }));
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(AtMost(1));
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onAuthorizationError("TEST_ME", "TIMEOUT", _)).Times(AtMost(1));
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(AtMost(1));
    EXPECT_TRUE(cblAuthorizationProvider->startAuthorization(""));

    // startAuthorization to return false when previous start authorization in progress.
    EXPECT_FALSE(cblAuthorizationProvider->startAuthorization(""));

    // Calling logout when current state is not AuthorizationState::AUTHORIZED
    EXPECT_FALSE(cblAuthorizationProvider->logout());

    cblAuthorizationProvider->shutdown();
}

}  // namespace unit
}  // namespace test
}  // namespace aace
