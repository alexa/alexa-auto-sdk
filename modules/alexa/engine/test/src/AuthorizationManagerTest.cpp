/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AVSCommon/SDKInterfaces/test/Storage/StubMiscStorage.h>
#include <AVSCommon/SDKInterfaces/test/MockAVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/Utils/Memory/Memory.h>

#include <RegistrationManager/RegistrationManager.h>
#include <RegistrationManager/CustomerDataManager.h>

#include <AACE/Engine/Alexa/AuthorizationManager.h>
#include <AACE/Engine/Alexa/AuthorizationManagerStorage.h>
#include <AACE/Test/Alexa/AlexaTestHelper.h>

using namespace aace::engine::alexa;
using namespace aace::test::alexa;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::sdkInterfaces::test;
using namespace ::testing;

class MockAuthorizationAdapterInterface : public AuthorizationAdapterInterface {
public:
    MOCK_METHOD0(deregister, void());
    MOCK_METHOD0(getAuthToken, std::string());
    MOCK_METHOD1(onAuthFailure, void(const std::string& token));
};

class MockAuthObserverInterface : public AuthObserverInterface {
public:
    MOCK_METHOD2(onAuthStateChange, void(State newState, Error error));
};

class AuthorizationManagerTest : public ::testing::Test {
public:
    void SetUp() override {
        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();
        m_miscStorage = storage::test::StubMiscStorage::create();
        m_directiveSequencer = std::make_shared<NiceMock<MockDirectiveSequencer>>();
        m_avsConnectionManager = std::make_shared<NiceMock<MockAVSConnectionManager>>();
        m_dataManager = std::make_shared<alexaClientSDK::registrationManager::CustomerDataManager>();
        m_registrationManager = std::make_shared<alexaClientSDK::registrationManager::RegistrationManager>(
            m_directiveSequencer, m_avsConnectionManager, m_dataManager);
    }

    void TearDown() override {
        if (m_directiveSequencer) {
            m_directiveSequencer->shutdown();
        }
        if (m_miscStorage) {
            m_miscStorage.reset();
        }
        if (m_registrationManager) {
            m_registrationManager.reset();
        }
        m_alexaMockFactory->shutdown();
    }

protected:
    std::shared_ptr<aace::engine::alexa::AuthorizationManager> createAuthorizationManager() {
        auto authorizationManagerStorage = AuthorizationManagerStorage::create(m_miscStorage);
        auto authorizationManager = AuthorizationManager::create(authorizationManagerStorage, m_dataManager);
        authorizationManager->setRegistrationManager(m_registrationManager);
        return authorizationManager;
    }

protected:
    /// Factory for getting the mocked components
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;

    /// The @c StubMiscStorage Interface used for the test.
    std::shared_ptr<storage::test::StubMiscStorage> m_miscStorage;

    /// The @c RegistrationManager used by the mocked classes
    std::shared_ptr<alexaClientSDK::registrationManager::RegistrationManager> m_registrationManager;

    /// The @c DirectiveSequencer used by the mocked classes
    std::shared_ptr<MockDirectiveSequencer> m_directiveSequencer;

    /// The @c AVSConnectionManager used by the mocked classes
    std::shared_ptr<MockAVSConnectionManager> m_avsConnectionManager;

    /// Data manager is used to call @c clearData() on every dataHandler.
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> m_dataManager;
};

TEST_F(AuthorizationManagerTest, test_createAuthorizationManagerWithInvalidParameters) {
    auto authorizationManagerStorageInvalid = AuthorizationManagerStorage::create(nullptr);
    ASSERT_EQ(authorizationManagerStorageInvalid, nullptr) << "AuthorizationManager pointer is not null!";

    auto authorizationManager = AuthorizationManager::create(nullptr, m_alexaMockFactory->getCustomerDataManagerMock());
    ASSERT_EQ(authorizationManager, nullptr) << "AuthorizationManager pointer is not null!";

    auto authorizationManagerStorageValid = AuthorizationManagerStorage::create(m_miscStorage);
    auto authorizationManager1 = AuthorizationManager::create(authorizationManagerStorageValid, nullptr);
    ASSERT_EQ(authorizationManager1, nullptr) << "AuthorizationManager pointer is not null!";
}

TEST_F(AuthorizationManagerTest, test_verifyRegistrationAndStartAuthorizationCornerCases) {
    auto authorizationManager = createAuthorizationManager();
    ASSERT_NE(authorizationManager, nullptr) << "AuthorizationManager pointer expected to be not null";

    // Registering adapter with empty service
    auto emtpyAdapter = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();
    authorizationManager->registerAuthorizationAdapter("", emtpyAdapter);

    auto startAuthorizationResult = authorizationManager->startAuthorization("test");
    ASSERT_EQ(startAuthorizationResult, AuthorizationManager::StartAuthorizationResult::FAILED)
        << "StartAuthorizationResult expected to be FAILED";

    auto validAdapter = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();
    authorizationManager->registerAuthorizationAdapter("test", validAdapter);

    // Calling startAuthorization with emtpy service name
    startAuthorizationResult = authorizationManager->startAuthorization("");
    ASSERT_EQ(startAuthorizationResult, AuthorizationManager::StartAuthorizationResult::FAILED)
        << "StartAuthorizationResult expected to be FAILED";
}

TEST_F(AuthorizationManagerTest, test_simpleHappyCase) {
    auto authorizationManager = createAuthorizationManager();
    ASSERT_NE(authorizationManager, nullptr) << "AuthorizationManager pointer expected to be not null";

    auto adapter1 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();
    auto adapter2 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();

    authorizationManager->registerAuthorizationAdapter("test1", adapter1);
    authorizationManager->registerAuthorizationAdapter("test2", adapter2);

    auto startAuthorizationResult = authorizationManager->startAuthorization("test1");
    ASSERT_EQ(startAuthorizationResult, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";

    authorizationManager->authStateChanged(
        "test1", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS);

    EXPECT_CALL(*adapter1, deregister()).Times(1);

    auto logoutResult = authorizationManager->logout("test1");
    ASSERT_EQ(logoutResult, true) << "StartAuthorizationResult expected to be true";
}

TEST_F(AuthorizationManagerTest, test_testingSuccessiveAuthorizationsHappyCase) {
    auto authorizationManager = createAuthorizationManager();
    ASSERT_NE(authorizationManager, nullptr) << "AuthorizationManager pointer expected to be not null";

    auto adapter1 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();
    auto adapter2 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();

    authorizationManager->registerAuthorizationAdapter("test1", adapter1);
    authorizationManager->registerAuthorizationAdapter("test2", adapter2);

    auto startAuthorizationResult1 = authorizationManager->startAuthorization("test1");
    ASSERT_EQ(startAuthorizationResult1, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";

    authorizationManager->authStateChanged(
        "test1", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS);

    EXPECT_CALL(*adapter1, deregister()).Times(1);

    auto startAuthorizationResult2 = authorizationManager->startAuthorization("test2");
    ASSERT_EQ(startAuthorizationResult2, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";

    authorizationManager->authStateChanged(
        "test2", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS);

    EXPECT_CALL(*adapter2, deregister()).Times(1);

    startAuthorizationResult1 = authorizationManager->startAuthorization("test1");
    ASSERT_EQ(startAuthorizationResult1, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";
}

TEST_F(AuthorizationManagerTest, test_successiveAuthorizationsWithoutAuthStateChange) {
    auto authorizationManager = createAuthorizationManager();
    ASSERT_NE(authorizationManager, nullptr) << "AuthorizationManager pointer expected to be not null";

    auto adapter1 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();
    auto adapter2 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();

    authorizationManager->registerAuthorizationAdapter("test1", adapter1);
    authorizationManager->registerAuthorizationAdapter("test2", adapter2);

    auto startAuthorizationResult1 = authorizationManager->startAuthorization("test1");
    ASSERT_EQ(startAuthorizationResult1, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";

    // Expect deregister call.
    EXPECT_CALL(*adapter1, deregister()).Times(1);

    auto startAuthorizationResult2 = authorizationManager->startAuthorization("test2");
    ASSERT_EQ(startAuthorizationResult2, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";

    authorizationManager->authStateChanged(
        "test2", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS);

    EXPECT_CALL(*adapter2, deregister()).Times(1);

    startAuthorizationResult1 = authorizationManager->startAuthorization("test1");
    ASSERT_EQ(startAuthorizationResult1, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";
}

TEST_F(AuthorizationManagerTest, test_verifyHappyPathAuthDelegateCalls) {
    auto authorizationManager = createAuthorizationManager();
    ASSERT_NE(authorizationManager, nullptr) << "AuthorizationManager pointer expected to be not null";

    auto authDelegate = authorizationManager->getAuthInterface();
    ASSERT_NE(authDelegate, nullptr) << "AuthDelegate pointer expected to be not null";

    auto mockAuthObserver = std::make_shared<StrictMock<MockAuthObserverInterface>>();
    EXPECT_CALL(
        *mockAuthObserver,
        onAuthStateChange(AuthObserverInterface::State::UNINITIALIZED, AuthObserverInterface::Error::SUCCESS))
        .Times(1);
    authDelegate->addAuthObserver(mockAuthObserver);  // Triggers onAuthStateChange with initial state.

    auto adapter = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();

    authorizationManager->registerAuthorizationAdapter("test1", adapter);

    auto startAuthorizationResult = authorizationManager->startAuthorization("test1");
    ASSERT_EQ(startAuthorizationResult, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";

    EXPECT_CALL(
        *mockAuthObserver,
        onAuthStateChange(AuthObserverInterface::State::REFRESHED, AuthObserverInterface::Error::SUCCESS))
        .Times(1);

    authorizationManager->authStateChanged(
        "test1", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS);

    EXPECT_CALL(*adapter, getAuthToken()).WillOnce(WithoutArgs(Invoke([]() { return "TestAuthToken"; })));
    EXPECT_EQ(authDelegate->getAuthToken(), "TestAuthToken") << "Auth token not as expected";

    EXPECT_CALL(
        *mockAuthObserver,
        onAuthStateChange(AuthObserverInterface::State::UNINITIALIZED, AuthObserverInterface::Error::SUCCESS))
        .Times(1);
    EXPECT_CALL(*adapter, deregister()).Times(1);

    auto logoutResult = authorizationManager->logout("test1");
    ASSERT_EQ(logoutResult, true) << "StartAuthorizationResult expected to be true";
}

TEST_F(AuthorizationManagerTest, test_verifyLogoutCornerCases) {
    auto authorizationManager = createAuthorizationManager();
    ASSERT_NE(authorizationManager, nullptr) << "AuthorizationManager pointer expected to be not null";

    auto adapter1 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();
    auto adapter2 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();

    authorizationManager->registerAuthorizationAdapter("test1", adapter1);
    authorizationManager->registerAuthorizationAdapter("test2", adapter2);

    auto startAuthorizationResult1 = authorizationManager->startAuthorization("test1");
    ASSERT_EQ(startAuthorizationResult1, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";

    authorizationManager->authStateChanged(
        "test1", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS);

    auto emptyServiceLogoutResult = authorizationManager->logout("");
    ASSERT_EQ(emptyServiceLogoutResult, false) << "logout expected to be false";

    auto invalidServiceLogoutResult = authorizationManager->logout("notRegistered");
    ASSERT_EQ(invalidServiceLogoutResult, false) << "logout expected to be false";

    auto notActiveServiceLogoutResult = authorizationManager->logout("test2");
    ASSERT_EQ(notActiveServiceLogoutResult, false) << "logout expected to be false";

    EXPECT_CALL(*adapter1, deregister()).Times(1);

    auto activeServiceLogoutResult = authorizationManager->logout("test1");
    ASSERT_EQ(activeServiceLogoutResult, true) << "logout expected to be true";
}

TEST_F(AuthorizationManagerTest, test_verifyLogoutWithoutSuccessfulAuthorization) {
    auto authorizationManager = createAuthorizationManager();
    ASSERT_NE(authorizationManager, nullptr) << "AuthorizationManager pointer expected to be not null";

    auto adapter1 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();
    auto adapter2 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();

    authorizationManager->registerAuthorizationAdapter("test1", adapter1);
    authorizationManager->registerAuthorizationAdapter("test2", adapter2);

    auto startAuthorizationResult1 = authorizationManager->startAuthorization("test1");
    ASSERT_EQ(startAuthorizationResult1, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";

    // Auth state changed for other adapter (test2) should not impact the adapter (test1)
    authorizationManager->authStateChanged(
        "test2", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS);

    // Expect a call to deregister irrespective of the previous service was authorized or not.
    EXPECT_CALL(*adapter1, deregister()).Times(1);

    auto activeServiceLogoutResult = authorizationManager->logout("test1");
    ASSERT_EQ(activeServiceLogoutResult, true) << "logout expected to be true";

    auto startAuthorizationResult2 = authorizationManager->startAuthorization("test2");
    ASSERT_EQ(startAuthorizationResult2, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";
}

TEST_F(AuthorizationManagerTest, test_verifyRegisteringDuplicatesOverwritesPreviousAdapter) {
    auto authorizationManager = createAuthorizationManager();
    ASSERT_NE(authorizationManager, nullptr) << "AuthorizationManager pointer expected to be not null";

    auto adapter1 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();
    auto adapter2 = std::make_shared<StrictMock<MockAuthorizationAdapterInterface>>();

    authorizationManager->registerAuthorizationAdapter("test", adapter1);
    authorizationManager->registerAuthorizationAdapter("test", adapter2);

    auto startAuthorizationResult = authorizationManager->startAuthorization("test");
    ASSERT_EQ(startAuthorizationResult, AuthorizationManager::StartAuthorizationResult::AUTHORIZE)
        << "StartAuthorizationResult expected to be AUTHORIZE";

    authorizationManager->authStateChanged(
        "test", AuthorizationManagerInterface::State::REFRESHED, AuthorizationManagerInterface::Error::SUCCESS);

    EXPECT_CALL(*adapter2, deregister()).Times(1);

    auto logoutResult = authorizationManager->logout("test");
    ASSERT_EQ(logoutResult, true) << "StartAuthorizationResult expected to be true";
}
