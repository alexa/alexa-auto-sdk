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

#include "AACE/Alexa/AlexaClient.h"
#include "AACE/Engine/Alexa/AlexaClientEngineImpl.h"

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

class MockAlexaClientPlatformInterface : public aace::alexa::AlexaClient {
public:
    MOCK_METHOD1(dialogStateChanged, 
        void(aace::alexa::AlexaClient::DialogState state));
    MOCK_METHOD2(authStateChanged, 
        void(aace::alexa::AlexaClient::AuthState, aace::alexa::AlexaClient::AuthError));
    MOCK_METHOD2(connectionStatusChanged, 
        void(aace::alexa::AlexaClient::ConnectionStatus, aace::alexa::AlexaClient::ConnectionChangedReason));
};

class AlexaClientEngineImplTest : public ::testing::Test {
public:

    void SetUp() override {
        if (DEBUG){
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }
        m_mockAlexaClientPlatformInterface = std::make_shared<testing::StrictMock<MockAlexaClientPlatformInterface>>();
        m_alexaClientEngineImpl = aace::engine::alexa::AlexaClientEngineImpl::create( 
            m_mockAlexaClientPlatformInterface);
        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::AlexaClientEngineImpl> m_alexaClientEngineImpl;
    std::shared_ptr<aace::test::unit::MockAlexaClientPlatformInterface> m_mockAlexaClientPlatformInterface;
};

/**
 * Test create() with valid AlexaClientPlatformInterface
 */
TEST_F( AlexaClientEngineImplTest, create ) {
    ASSERT_NE(nullptr, m_alexaClientEngineImpl) << "AlexaClient pointer expected to be not null ";
}

/**
 * Test create() with nullptr
 */
TEST_F( AlexaClientEngineImplTest, createWithNull ) {
    EXPECT_EQ(nullptr, nullptr) << "Pointer is expected to be null";
}

/**
 * Test dialogStateChanged() for IDLE callback
 */
TEST_F( AlexaClientEngineImplTest, testdialogStateChangedCallbackForIdle ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, dialogStateChanged(aace::alexa::AlexaClient::DialogState::IDLE));
    m_alexaClientEngineImpl->onDialogUXStateChanged( 
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::IDLE );
}

/**
 * Test dialogStateChanged() for LISTENING callback
 */
TEST_F( AlexaClientEngineImplTest, testdialogStateChangedCallbackForListening ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, dialogStateChanged(aace::alexa::AlexaClient::DialogState::LISTENING));
    m_alexaClientEngineImpl->onDialogUXStateChanged( 
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::LISTENING );
}

/**
 * Test dialogStateChanged() for THINKING callback
 */
TEST_F( AlexaClientEngineImplTest, testdialogStateChangedCallbackForThinking ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, dialogStateChanged(aace::alexa::AlexaClient::DialogState::THINKING));
    m_alexaClientEngineImpl->onDialogUXStateChanged( 
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::THINKING );
}

/**
 * Test dialogStateChanged() for SPEAKING callback
 */
TEST_F( AlexaClientEngineImplTest, testdialogStateChangedCallbackForSpeaking ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, dialogStateChanged(aace::alexa::AlexaClient::DialogState::SPEAKING));
    m_alexaClientEngineImpl->onDialogUXStateChanged( 
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::SPEAKING );
}

/**
 * Test authStateChanged() for sucessfull case callback
 */
TEST_F( AlexaClientEngineImplTest, testauthStateChangedCallbackForSuccessCase ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, 
        authStateChanged(aace::alexa::AlexaClient::AuthState::UNINITIALIZED,
        aace::alexa::AlexaClient::AuthError::NO_ERROR));
    m_alexaClientEngineImpl->onAuthStateChange( 
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::UNINITIALIZED,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS);
}

/**
 * Test authStateChanged() for random case callback
 */
TEST_F( AlexaClientEngineImplTest, testauthStateChangedCallbackForUnsuccessCase ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, 
        authStateChanged(aace::alexa::AlexaClient::AuthState::REFRESHED,
        aace::alexa::AlexaClient::AuthError::UNAUTHORIZED_CLIENT));
    m_alexaClientEngineImpl->onAuthStateChange( 
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::UNAUTHORIZED_CLIENT );
}

/**
 * Test connectionStatusChanged() for sucessfull connected callback
 */
TEST_F( AlexaClientEngineImplTest, testconnectionStatusChangedCallbackForConnectedCase ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, 
        connectionStatusChanged(aace::alexa::AlexaClient::ConnectionStatus::CONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_ENDPOINT_CHANGED));
    m_alexaClientEngineImpl->onConnectionStatusChanged( 
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::SERVER_ENDPOINT_CHANGED);
}

/**
 * Test connectionStatusChanged() some random boundry test.
 */
TEST_F( AlexaClientEngineImplTest, testconnectionStatusChangedCallbackForRandomCase1 ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, 
        connectionStatusChanged(aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::ACL_CLIENT_REQUEST));
    m_alexaClientEngineImpl->onConnectionStatusChanged( 
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::ACL_CLIENT_REQUEST);
}

/**
 * Test connectionStatusChanged() some random boundry test.
 */
TEST_F( AlexaClientEngineImplTest, testconnectionStatusChangedCallbackForRandomCase2 ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, 
        connectionStatusChanged(aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::PING_TIMEDOUT));
    m_alexaClientEngineImpl->onConnectionStatusChanged( 
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::PING_TIMEDOUT);
}

/**
 * Test connectionStatusChanged() some random test.
 */
TEST_F( AlexaClientEngineImplTest, testconnectionStatusChangedCallbackForRandomCase3 ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, 
        connectionStatusChanged(aace::alexa::AlexaClient::ConnectionStatus::DISCONNECTED,
        aace::alexa::AlexaClient::ConnectionChangedReason::SERVER_SIDE_DISCONNECT));
    m_alexaClientEngineImpl->onConnectionStatusChanged( 
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::SERVER_SIDE_DISCONNECT);
}

/**
 * Test connectionStatusChanged() some random boundry test.
 */
TEST_F( AlexaClientEngineImplTest, testconnectionStatusChangedCallbackForRandomCase4 ){
    EXPECT_CALL(*m_mockAlexaClientPlatformInterface, 
        connectionStatusChanged(aace::alexa::AlexaClient::ConnectionStatus::PENDING,
        aace::alexa::AlexaClient::ConnectionChangedReason::INTERNAL_ERROR));
    m_alexaClientEngineImpl->onConnectionStatusChanged( 
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::PENDING,
        alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason::INTERNAL_ERROR);
}

}  // namespace unit
}  // namespace test
}  // namespace aace

