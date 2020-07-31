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

#ifndef AACE_TEST_AVS_MOCK_ENDPOINT_REGISTRATION_MANAGER_INTERFACE_H
#define AACE_TEST_AVS_MOCK_ENDPOINT_REGISTRATION_MANAGER_INTERFACE_H

#include <AVSCommon/SDKInterfaces/Endpoints/EndpointRegistrationManagerInterface.h>

namespace aace {
namespace test {
namespace avs {

class MockEndpointRegistrationManagerInterface
        : public alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointRegistrationManagerInterface {
public:
    MOCK_METHOD1(
        registerEndpoint,
        std::future<RegistrationResult>(
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointInterface> endpoint));
    MOCK_METHOD1(
        addObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointRegistrationObserverInterface>
                 observer));
    MOCK_METHOD1(
        removeObserver,
        void(const std::shared_ptr<
             alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointRegistrationObserverInterface>& observer));
};

}  // namespace avs
}  // namespace test
}  // namespace aace

#endif  //AACE_TEST_AVS_MOCK_ENDPOINT_REGISTRATION_MANAGER_INTERFACE_H
