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

#ifndef AACE_ENGINE_TEST_ALEXA_MOCK_AUTHORIZATION_MANAGER_H
#define AACE_ENGINE_TEST_ALEXA_MOCK_AUTHORIZATION_MANAGER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <AACE/Engine/Alexa/AuthorizationManagerInterface.h>

namespace aace {
namespace test {
namespace alexa {

class MockAuthorizationManager : public engine::alexa::AuthorizationManagerInterface {
public:
    MOCK_METHOD2(
        registerAuthorizationAdapter,
        void(const std::string& service, std::shared_ptr<engine::alexa::AuthorizationAdapterInterface> adapter));
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

}  // namespace alexa
}  // namespace test
}  // namespace aace

#endif  // AACE_ENGINE_TEST_ALEXA_MOCK_AUTHORIZATION_MANAGER_H
