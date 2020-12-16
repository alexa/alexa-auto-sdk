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
#include <AACE/Engine/Alexa/AuthProviderEngineImpl.h>

using namespace aace::test::alexa;
using namespace aace::engine::alexa;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::sdkInterfaces::test;
using namespace ::testing;

class MockAuthorizationManager : public AuthorizationManager {
public:
    MOCK_METHOD2(
        registerAuthorizationAdapter,
        void(const std::string& service, std::shared_ptr<AuthorizationAdapterInterface> adapter));
    MOCK_METHOD1(startAuthorization, StartAuthorizationResult(const std::string& service));
    MOCK_METHOD3(authStateChanged, void(const std::string& service, State state, Error reason));
    MOCK_METHOD1(logout, bool(const std::string& service));
    MOCK_METHOD1(
        addAuthObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer));
    MOCK_METHOD1(
        removeAuthObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer));
    MOCK_METHOD0(getAuthToken, std::string());
    MOCK_METHOD1(onAuthFailure, void(const std::string& token));
    MOCK_METHOD0(clearData, void());
};

class AuthProviderEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();
        m_miscStorage = storage::test::StubMiscStorage::create();
        m_directiveSequencer = std::make_shared<NiceMock<MockDirectiveSequencer>>();
        m_avsConnectionManager = std::make_shared<NiceMock<MockAVSConnectionManager>>();
        m_dataManager = std::make_shared<alexaClientSDK::registrationManager::CustomerDataManager>();
        m_registrationManager = std::make_shared<alexaClientSDK::registrationManager::RegistrationManager>(
            m_directiveSequencer, m_avsConnectionManager, m_dataManager);
        auto authorizationManagerStorage = AuthorizationManagerStorage::create(m_miscStorage);
        m_authorizationManager = AuthorizationManager::create(authorizationManagerStorage, m_dataManager);
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
    std::shared_ptr<aace::engine::alexa::AuthProviderEngineImpl> createAuthProviderEngineImpl() {
        return aace::engine::alexa::AuthProviderEngineImpl::create(
            m_alexaMockFactory->getAuthProviderMock(), m_authorizationManager);
    }

protected:
    /// Factory for getting the mocked components
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;

    /// The @c StubMiscStorage Interface used for the test.
    std::shared_ptr<storage::test::StubMiscStorage> m_miscStorage;

    /// The @c DirectiveSequencer used by the mocked classes
    std::shared_ptr<MockDirectiveSequencer> m_directiveSequencer;

    /// The @c AVSConnectionManager used by the mocked classes
    std::shared_ptr<MockAVSConnectionManager> m_avsConnectionManager;

    /// Data manager is used to call @c clearData() on every dataHandler.
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> m_dataManager;

    /// The @c RegistrationManager used by the mocked classes
    std::shared_ptr<alexaClientSDK::registrationManager::RegistrationManager> m_registrationManager;

    /// The @c AuthorizationManager used by the mocked classes
    std::shared_ptr<AuthorizationManager> m_authorizationManager;
};

TEST_F(AuthProviderEngineImplTest, create) {
    auto authProviderEngineImpl = createAuthProviderEngineImpl();
    ASSERT_NE(authProviderEngineImpl, nullptr) << "AuthProviderEngineImpl pointer is null!";

    authProviderEngineImpl->shutdown();
}

TEST_F(AuthProviderEngineImplTest, createWithPlatformInterfaceAsNull) {
    auto authProviderEngineImpl = aace::engine::alexa::AuthProviderEngineImpl::create(nullptr, m_authorizationManager);
    ASSERT_EQ(authProviderEngineImpl, nullptr) << "AuthProviderEngineImpl pointer expected to be null!";

    auto authProviderEngineImpl1 =
        aace::engine::alexa::AuthProviderEngineImpl::create(m_alexaMockFactory->getAuthProviderMock(), nullptr);
    ASSERT_EQ(authProviderEngineImpl1, nullptr) << "AuthProviderEngineImpl pointer expected to be null!";
}
