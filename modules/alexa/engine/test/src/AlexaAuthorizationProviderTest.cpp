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

#include <AACE/Engine/Alexa/AlexaAuthorizationProvider.h>
#include <AACE/Engine/Alexa/AuthorizationManagerInterface.h>

namespace aace {
namespace test {
namespace unit {

/// Plenty of timeout to wait for async task to run
static std::chrono::seconds TIMEOUT(2);

using namespace ::testing;
using namespace aace::engine::alexa;
using namespace aace::engine::authorization;
using namespace aace::test::alexa;
using namespace aace::test::alexa;
using namespace aace::test::core;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;

class AlexaAuthorizationProviderTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockAuthorizationManager = std::make_shared<StrictMock<MockAuthorizationManager>>();
        m_mockAuthorizationProviderListener = std::make_shared<StrictMock<MockAuthorizationProviderListener>>();
    }

    void TearDown() override {
        if (m_mockAuthorizationManager) {
            m_mockAuthorizationManager.reset();
        }
        if (m_mockAuthorizationProviderListener) {
            m_mockAuthorizationProviderListener.reset();
        }
    }

protected:
    std::shared_ptr<aace::engine::alexa::AlexaAuthorizationProvider> createAlexaAuthorizationProvider() {
        auto alexaAuthorizationProvider =
            aace::engine::alexa::AlexaAuthorizationProvider::create("TEST_ME", m_mockAuthorizationManager);
        alexaAuthorizationProvider->setListener(m_mockAuthorizationProviderListener);
        return alexaAuthorizationProvider;
    }

protected:
    /// The @c AuthorizationManager used by the mocked classes
    std::shared_ptr<MockAuthorizationManager> m_mockAuthorizationManager;

    /// The mocked @c AuthorizationProviderListenerInterface
    std::shared_ptr<MockAuthorizationProviderListener> m_mockAuthorizationProviderListener;
};

TEST_F(AlexaAuthorizationProviderTest, createWithNullParameters) {
    auto alexaAuthorizationProvider =
        aace::engine::alexa::AlexaAuthorizationProvider::create("", m_mockAuthorizationManager);
    ASSERT_EQ(alexaAuthorizationProvider, nullptr) << "AlexaAuthorizationProvider pointer expected to be null!";

    auto alexaAuthorizationProvider1 = aace::engine::alexa::AlexaAuthorizationProvider::create("testMe", nullptr);
    ASSERT_EQ(alexaAuthorizationProvider1, nullptr) << "AlexaAuthorizationProvider pointer expected to be null!";
}

TEST_F(AlexaAuthorizationProviderTest, simpleHappyUsecase) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto alexaAuthorizationProvider = createAlexaAuthorizationProvider();
    ASSERT_NE(alexaAuthorizationProvider, nullptr) << "AlexaAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onEventReceived("TEST_ME", R"({"type":"requestAuthorization"})"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([alexaAuthorizationProvider]() -> void {
            alexaAuthorizationProvider->sendEvent(R"({
                                        "type":"authStateChangeEvent",
                                        "payload": {
                                            "state":"AUTHORIZED"
                                        }
                                    })");
        }));
    EXPECT_CALL(
        *m_mockAuthorizationManager,
        authStateChanged(
            "TEST_ME", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS))
        .Times(1);

    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() -> void { waitEvent.wakeUp(); }));

    EXPECT_TRUE(alexaAuthorizationProvider->startAuthorization(""));
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));

    EXPECT_CALL(*m_mockAuthorizationProviderListener, onGetAuthorizationData("TEST_ME", "accessToken"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent, alexaAuthorizationProvider]() -> std::string {
            std::string accessToken = R"({"accessToken":"MY_TEST_TOKEN"})";
            waitEvent.wakeUp();
            return accessToken;
        }));
    auto authToken = alexaAuthorizationProvider->getAuthToken();
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
    EXPECT_EQ(authToken, "MY_TEST_TOKEN");

    alexaAuthorizationProvider->shutdown();
}

TEST_F(AlexaAuthorizationProviderTest, subsequentStartAndStop) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto alexaAuthorizationProvider = createAlexaAuthorizationProvider();
    ASSERT_NE(alexaAuthorizationProvider, nullptr) << "AlexaAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onEventReceived("TEST_ME", "{\"type\":\"requestAuthorization\"}"))
        .Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() -> void { waitEvent.wakeUp(); }));

    EXPECT_TRUE(alexaAuthorizationProvider->startAuthorization(""));
    EXPECT_TRUE(alexaAuthorizationProvider->cancelAuthorization());

    EXPECT_TRUE(waitEvent.wait(TIMEOUT));

    alexaAuthorizationProvider->shutdown();
}

TEST_F(AlexaAuthorizationProviderTest, sendEventWithUnauthorized) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent1, waitEvent2;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto alexaAuthorizationProvider = createAlexaAuthorizationProvider();
    ASSERT_NE(alexaAuthorizationProvider, nullptr) << "AlexaAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onEventReceived("TEST_ME", R"({"type":"requestAuthorization"})"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([alexaAuthorizationProvider]() -> void {
            EXPECT_TRUE(alexaAuthorizationProvider->sendEvent(R"({
                                        "type":"authStateChangeEvent",
                                        "payload": {
                                            "state":"UNAUTHORIZED"
                                        }
                                    })"));
        }));
    EXPECT_CALL(
        *m_mockAuthorizationManager,
        authStateChanged(
            "TEST_ME",
            AuthorizationManagerInterface::State::UNINITIALIZED,
            AuthorizationManagerInterface::Error::SUCCESS))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent1]() -> void { waitEvent1.wakeUp(); }));

    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent2]() -> void { waitEvent2.wakeUp(); }));

    EXPECT_TRUE(alexaAuthorizationProvider->startAuthorization(""));
    EXPECT_TRUE(waitEvent1.wait(TIMEOUT));

    EXPECT_TRUE(alexaAuthorizationProvider->cancelAuthorization());
    EXPECT_TRUE(waitEvent2.wait(TIMEOUT));

    alexaAuthorizationProvider->shutdown();
}

TEST_F(AlexaAuthorizationProviderTest, logoutHappyCase) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent1, waitEvent2;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto alexaAuthorizationProvider = createAlexaAuthorizationProvider();
    ASSERT_NE(alexaAuthorizationProvider, nullptr) << "AlexaAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, logout("TEST_ME"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([alexaAuthorizationProvider]() -> bool {
            alexaAuthorizationProvider->deregister();
            return true;
        }));
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onEventReceived("TEST_ME", R"({"type":"logout"})")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent2]() -> void { waitEvent2.wakeUp(); }));

    EXPECT_TRUE(alexaAuthorizationProvider->logout());
    EXPECT_TRUE(waitEvent2.wait(TIMEOUT));

    alexaAuthorizationProvider->shutdown();
}

TEST_F(AlexaAuthorizationProviderTest, passingValidAndInvalidJsonString) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent1, waitEvent2;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto alexaAuthorizationProvider = createAlexaAuthorizationProvider();
    ASSERT_NE(alexaAuthorizationProvider, nullptr) << "AlexaAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onEventReceived("TEST_ME", R"({"type":"requestAuthorization"})"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent1]() -> void { waitEvent1.wakeUp(); }));

    EXPECT_TRUE(alexaAuthorizationProvider->startAuthorization("ABC"));
    EXPECT_TRUE(waitEvent1.wait(TIMEOUT));

    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged(
            "TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent2]() -> void { waitEvent2.wakeUp(); }));

    EXPECT_TRUE(alexaAuthorizationProvider->cancelAuthorization());
    EXPECT_TRUE(waitEvent2.wait(TIMEOUT));

    waitEvent1.reset();
    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onEventReceived("TEST_ME", R"({"type":"requestAuthorization"})"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent1]() -> void { waitEvent1.wakeUp(); }));

    // Empty Json
    EXPECT_TRUE(alexaAuthorizationProvider->startAuthorization("{}"));
    EXPECT_TRUE(waitEvent1.wait(TIMEOUT));

    alexaAuthorizationProvider->shutdown();
}

TEST_F(AlexaAuthorizationProviderTest, sendEventWithInvalidPayload) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto alexaAuthorizationProvider = createAlexaAuthorizationProvider();
    ASSERT_NE(alexaAuthorizationProvider, nullptr) << "AlexaAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onEventReceived("TEST_ME", R"({"type":"requestAuthorization"})"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent, alexaAuthorizationProvider]() -> void {
            EXPECT_TRUE(alexaAuthorizationProvider->sendEvent(R"({
                                        "type":"invalid",
                                        "payload": {
                                            "state":"AUTHORIZED"
                                        }
                                    })"));
            waitEvent.wakeUp();
        }));
    EXPECT_CALL(
        *m_mockAuthorizationManager,
        authStateChanged(
            "TEST_ME", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS))
        .Times(0);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED))
        .Times(0);

    EXPECT_TRUE(alexaAuthorizationProvider->startAuthorization(""));
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));

    alexaAuthorizationProvider->shutdown();
}

TEST_F(AlexaAuthorizationProviderTest, onGetAuthorizationDataReturningInvalidJson) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto alexaAuthorizationProvider = createAlexaAuthorizationProvider();
    ASSERT_NE(alexaAuthorizationProvider, nullptr) << "AlexaAuthorizationProvider pointer expected to be not null!";

    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME")).Times(1);
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onEventReceived("TEST_ME", R"({"type":"requestAuthorization"})"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([alexaAuthorizationProvider]() -> void {
            EXPECT_TRUE(alexaAuthorizationProvider->sendEvent(R"({
                                        "type":"authStateChangeEvent",
                                        "payload": {
                                            "state":"AUTHORIZED"
                                        }
                                    })"));
        }));
    EXPECT_CALL(
        *m_mockAuthorizationManager,
        authStateChanged(
            "TEST_ME", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS))
        .Times(1);

    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() -> void { waitEvent.wakeUp(); }));

    EXPECT_TRUE(alexaAuthorizationProvider->startAuthorization("ANY_STRING"));
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));

    EXPECT_CALL(*m_mockAuthorizationProviderListener, onGetAuthorizationData("TEST_ME", "accessToken"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent, alexaAuthorizationProvider]() -> std::string {
            std::string accessToken = R"({"access":"MY_TEST_TOKEN"})";  // Invalid JSON
            waitEvent.wakeUp();
            return accessToken;
        }));
    auto authToken = alexaAuthorizationProvider->getAuthToken();
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
    EXPECT_EQ(authToken, "");
    alexaAuthorizationProvider->shutdown();
}

TEST_F(AlexaAuthorizationProviderTest, testPossibleFailurePaths) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;
    EXPECT_CALL(*m_mockAuthorizationManager, registerAuthorizationAdapter("TEST_ME", ::testing::_)).Times(1);
    auto alexaAuthorizationProvider = createAlexaAuthorizationProvider();
    ASSERT_NE(alexaAuthorizationProvider, nullptr) << "AlexaAuthorizationProvider pointer expected to be not null!";

    // Cancel called before startAuthorization
    EXPECT_FALSE(alexaAuthorizationProvider->cancelAuthorization());

    // AuthorizationManager returning StartAuthorizationResult::FAILED
    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([]() -> AuthorizationManagerInterface::StartAuthorizationResult {
            return AuthorizationManagerInterface::StartAuthorizationResult::FAILED;
        }));
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onAuthorizationError("TEST_ME", "START_AUTHORIZATION_FAILED", _))
        .Times(1);
    EXPECT_TRUE(alexaAuthorizationProvider->startAuthorization(""));

    waitEvent.reset();
    EXPECT_CALL(*m_mockAuthorizationManager, startAuthorization("TEST_ME"))
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([]() -> AuthorizationManagerInterface::StartAuthorizationResult {
            return AuthorizationManagerInterface::StartAuthorizationResult::REAUTHORIZE;
        }));
    EXPECT_CALL(
        *m_mockAuthorizationProviderListener,
        onAuthorizationStateChanged("TEST_ME", AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING))
        .Times(1);
    EXPECT_CALL(*m_mockAuthorizationProviderListener, onEventReceived("TEST_ME", R"({"type":"requestAuthorization"})"))
        .Times(1);
    EXPECT_TRUE(alexaAuthorizationProvider->startAuthorization(""));

    // startAuthorization to return false when previous start authorization in progress.
    EXPECT_FALSE(alexaAuthorizationProvider->startAuthorization(""));

    // Calling logout when current state is not AuthorizationState::AUTHORIZED
    EXPECT_FALSE(alexaAuthorizationProvider->logout());

    alexaAuthorizationProvider->shutdown();
}

}  // namespace unit
}  // namespace test
}  // namespace aace
