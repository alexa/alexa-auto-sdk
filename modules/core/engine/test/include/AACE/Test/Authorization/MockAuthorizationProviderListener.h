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

#ifndef AACE_ENGINE_TEST_AUTHORIZATION_MOCK_AUTHORIZATION_PROVIDER_H
#define AACE_ENGINE_TEST_AUTHORIZATION_MOCK_AUTHORIZATION_PROVIDER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <AACE/Engine/Authorization/AuthorizationProviderListenerInterface.h>

namespace aace {
namespace test {
namespace core {

class MockAuthorizationProviderListener : public engine::authorization::AuthorizationProviderListenerInterface {
public:
    MOCK_METHOD2(onAuthorizationStateChanged, void(const std::string& service, AuthorizationState state));
    MOCK_METHOD3(onAuthorizationError, void(const std::string&, const std::string&, const std::string&));
    MOCK_METHOD2(onEventReceived, void(const std::string& service, const std::string& event));
    MOCK_METHOD2(onGetAuthorizationData, std::string(const std::string& service, const std::string& key));
    MOCK_METHOD3(
        onSetAuthorizationData,
        void(const std::string& service, const std::string& key, const std::string& data));
};

}  // namespace core
}  // namespace test
}  // namespace aace

#endif  // AACE_ENGINE_TEST_AUTHORIZATION_MOCK_AUTHORIZATION_PROVIDER_H
