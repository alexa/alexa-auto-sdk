/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>

#include <AACE/Test/Unit/Alexa/AlexaTestHelper.h>
#include "AACE/Engine/Alexa/AlexaClientEngineImpl.h"

using namespace aace::test::unit::alexa;

class AlexaClientEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();
    }

    void TearDown() override {
        m_alexaMockFactory->shutdown();
    }

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;
};

TEST_F(AlexaClientEngineImplTest, create) {
    auto alexaEngineClientImpl = aace::engine::alexa::AlexaClientEngineImpl::create(
        m_alexaMockFactory->getAlexaClientMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());
    ASSERT_NE(alexaEngineClientImpl, nullptr) << "AlexaClientEngineImpl pointer is null!";
}

TEST_F(AlexaClientEngineImplTest, createWithPlatformInterfaceAsNull) {
    auto alexaEngineClientImpl = aace::engine::alexa::AlexaClientEngineImpl::create(
        nullptr,
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());
    ASSERT_EQ(alexaEngineClientImpl, nullptr) << "AlexaClientEngineImpl pointer expected to be null!";
}

TEST_F(AlexaClientEngineImplTest, createWithFocusManagerAsNull) {
    auto alexaEngineClientImpl = aace::engine::alexa::AlexaClientEngineImpl::create(
        m_alexaMockFactory->getAlexaClientMock(), nullptr, m_alexaMockFactory->getFocusManagerInterfaceMock());
    ASSERT_EQ(alexaEngineClientImpl, nullptr) << "AlexaClientEngineImpl pointer expected to be null!";

    alexaEngineClientImpl = aace::engine::alexa::AlexaClientEngineImpl::create(
        m_alexaMockFactory->getAlexaClientMock(), m_alexaMockFactory->getFocusManagerInterfaceMock(), nullptr);
    ASSERT_EQ(alexaEngineClientImpl, nullptr) << "AlexaClientEngineImpl pointer expected to be null!";
}

TEST_F(AlexaClientEngineImplTest, verifyDialogStateCallbacks) {
    auto alexaEngineClientImpl = aace::engine::alexa::AlexaClientEngineImpl::create(
        m_alexaMockFactory->getAlexaClientMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());
    ASSERT_NE(alexaEngineClientImpl, nullptr) << "AlexaClientEngineImpl pointer is null!";

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(), dialogStateChanged(aace::alexa::AlexaClient::DialogState::IDLE));
    alexaEngineClientImpl->onDialogUXStateChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::IDLE);

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        dialogStateChanged(aace::alexa::AlexaClient::DialogState::LISTENING));
    alexaEngineClientImpl->onDialogUXStateChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::LISTENING);

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(), dialogStateChanged(aace::alexa::AlexaClient::DialogState::THINKING));
    alexaEngineClientImpl->onDialogUXStateChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::THINKING);

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(), dialogStateChanged(aace::alexa::AlexaClient::DialogState::SPEAKING));
    alexaEngineClientImpl->onDialogUXStateChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::SPEAKING);
}

TEST_F(AlexaClientEngineImplTest, verifyAuthStateCallbacks) {
    auto alexaEngineClientImpl = aace::engine::alexa::AlexaClientEngineImpl::create(
        m_alexaMockFactory->getAlexaClientMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());
    ASSERT_NE(alexaEngineClientImpl, nullptr) << "AlexaClientEngineImpl pointer is null!";

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        authStateChanged(
            aace::alexa::AlexaClient::AuthState::UNINITIALIZED, aace::alexa::AlexaClient::AuthError::NO_ERROR));
    alexaEngineClientImpl->onAuthStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::UNINITIALIZED,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS);

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        authStateChanged(
            aace::alexa::AlexaClient::AuthState::REFRESHED, aace::alexa::AlexaClient::AuthError::UNAUTHORIZED_CLIENT));
    alexaEngineClientImpl->onAuthStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::UNAUTHORIZED_CLIENT);
}

TEST_F(AlexaClientEngineImplTest, verifyConnectionStatusCallbacks) {
    auto alexaEngineClientImpl = aace::engine::alexa::AlexaClientEngineImpl::create(
        m_alexaMockFactory->getAlexaClientMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());
    ASSERT_NE(alexaEngineClientImpl, nullptr) << "AlexaClientEngineImpl pointer is null!";

    std::vector<alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::EngineConnectionStatus>
        engineStatuses1;
    engineStatuses1.emplace_back(
        alexaClientSDK::avsCommon::sdkInterfaces::ENGINE_TYPE_ALEXA_VOICE_SERVICES,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::
            SERVER_ENDPOINT_CHANGED,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED);

    std::vector<aace::alexa::AlexaClient::ConnectionStatusInfo> statusDetailed1;
    statusDetailed1.emplace_back(
        aace::alexa::AlexaClient::ConnectionType::AVS,
        aace::alexa::AlexaClient::ConnectionStatus::CONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED);

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::CONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED));
    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::CONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED,
            statusDetailed1));

    alexaEngineClientImpl->onConnectionStatusChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED,
        engineStatuses1);

    std::vector<alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::EngineConnectionStatus>
        engineStatuses2;
    engineStatuses2.emplace_back(
        alexaClientSDK::avsCommon::sdkInterfaces::ENGINE_TYPE_ALEXA_VOICE_SERVICES,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED);

    std::vector<aace::alexa::AlexaClient::ConnectionStatusInfo> statusDetailed2;
    statusDetailed2.emplace_back(
        aace::alexa::AlexaClient::ConnectionType::AVS,
        aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST);

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST));
    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST,
            statusDetailed2));
    alexaEngineClientImpl->onConnectionStatusChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED,
        engineStatuses2);

    std::vector<alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::EngineConnectionStatus>
        engineStatuses3;
    engineStatuses3.emplace_back(
        alexaClientSDK::avsCommon::sdkInterfaces::ENGINE_TYPE_ALEXA_VOICE_SERVICES,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::INTERNAL_ERROR,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::PENDING);

    std::vector<aace::alexa::AlexaClient::ConnectionStatusInfo> statusDetailed3;
    statusDetailed3.emplace_back(
        aace::alexa::AlexaClient::ConnectionType::AVS,
        aace::alexa::AlexaClient::ConnectionStatus::PENDING,
        aace::alexa::AlexaClient::ConnectionChangedReason::INTERNAL_ERROR);

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::PENDING,
            aace::alexa::AlexaClient::ConnectionChangedReason::INTERNAL_ERROR));
    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::PENDING,
            aace::alexa::AlexaClient::ConnectionChangedReason::INTERNAL_ERROR,
            statusDetailed3));
    alexaEngineClientImpl->onConnectionStatusChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::PENDING, engineStatuses3);

    std::vector<alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::EngineConnectionStatus>
        engineStatuses4;
    engineStatuses4.emplace_back(
        alexaClientSDK::avsCommon::sdkInterfaces::ENGINE_TYPE_ALEXA_VOICE_SERVICES,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::PING_TIMEDOUT,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED);

    std::vector<aace::alexa::AlexaClient::ConnectionStatusInfo> statusDetailed4;
    statusDetailed4.emplace_back(
        aace::alexa::AlexaClient::ConnectionType::AVS,
        aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::PING_TIMEDOUT);

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::PING_TIMEDOUT));
    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::PING_TIMEDOUT,
            statusDetailed4));
    alexaEngineClientImpl->onConnectionStatusChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED,
        engineStatuses4);

    //Running a repeated test to ensure last status and next status are not matching, otherwise engine would ignore it
    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::CONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED));
    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::CONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED,
            statusDetailed1));
    alexaEngineClientImpl->onConnectionStatusChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED,
        engineStatuses1);

    std::vector<alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::EngineConnectionStatus>
        engineStatuses5;
    engineStatuses5.emplace_back(
        alexaClientSDK::avsCommon::sdkInterfaces::ENGINE_TYPE_ALEXA_VOICE_SERVICES,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::
            SERVER_SIDE_DISCONNECT,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED);
    std::vector<aace::alexa::AlexaClient::ConnectionStatusInfo> statusDetailed5;
    statusDetailed5.emplace_back(
        aace::alexa::AlexaClient::ConnectionType::AVS,
        aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT);
    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT));
    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT,
            statusDetailed5));
    alexaEngineClientImpl->onConnectionStatusChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED,
        engineStatuses5);

    // Test connection status change on the local engine type
    std::vector<alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::EngineConnectionStatus>
        engineStatuses6;
    engineStatuses6.emplace_back(
        alexaClientSDK::avsCommon::sdkInterfaces::ENGINE_TYPE_ALEXA_VOICE_SERVICES,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::
            SERVER_SIDE_DISCONNECT,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED);
    engineStatuses6.emplace_back(
        aace::engine::alexa::ENGINE_TYPE_LOCAL_INT,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::NONE,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED);

    std::vector<aace::alexa::AlexaClient::ConnectionStatusInfo> statusDetailed6;
    statusDetailed6.emplace_back(
        aace::alexa::AlexaClient::ConnectionType::AVS,
        aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT);
    statusDetailed6.emplace_back(
        aace::alexa::AlexaClient::ConnectionType::LOCAL,
        aace::alexa::AlexaClient::ConnectionStatus::CONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::NONE);

    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::CONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::NONE));
    EXPECT_CALL(
        *m_alexaMockFactory->getAlexaClientMock(),
        connectionStatusChanged(
            aace::alexa::AlexaClient::ConnectionStatus::CONNECTED,
            aace::alexa::AlexaClient::ConnectionChangedReason::NONE,
            statusDetailed6));
    alexaEngineClientImpl->onConnectionStatusChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED,
        engineStatuses6);
}

TEST_F(AlexaClientEngineImplTest, verifyStopForegroundActivity) {
    auto alexaEngineClientImpl = aace::engine::alexa::AlexaClientEngineImpl::create(
        m_alexaMockFactory->getAlexaClientMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock(),
        m_alexaMockFactory->getFocusManagerInterfaceMock());
    ASSERT_NE(alexaEngineClientImpl, nullptr) << "AlexaClientEngineImpl pointer is null!";

    EXPECT_CALL(*m_alexaMockFactory->getFocusManagerInterfaceMock(), stopForegroundActivity()).Times(2);
    alexaEngineClientImpl->onStopForegroundActivity();
}
