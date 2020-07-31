/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_AVS_MOCK_AUTH_DELEGATE_INTERFACE_H
#define AACE_TEST_AVS_MOCK_AUTH_DELEGATE_INTERFACE_H

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>

namespace aace {
namespace test {
namespace avs {

class MockAuthDelegateInterface : public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface {
public:
    MOCK_METHOD1(
        addAuthObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer));
    MOCK_METHOD1(
        removeAuthObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer));
    MOCK_METHOD0(getAuthToken, std::string());
    MOCK_METHOD1(onAuthFailure, void(const std::string& token));
    MOCK_METHOD0(doShutdown, void());
};

}  // namespace avs
}  // namespace test
}  // namespace aace

#endif  //AACE_TEST_AVS_MOCK_AUTH_DELEGATE_INTERFACE_H
