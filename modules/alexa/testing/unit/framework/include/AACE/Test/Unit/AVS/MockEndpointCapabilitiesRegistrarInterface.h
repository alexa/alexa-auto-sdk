/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_UNIT_AVS_MOCK_ENDPOINT_CAPABILITIES_REGISTRAR_INTERFACE_H
#define AACE_TEST_UNIT_AVS_MOCK_ENDPOINT_CAPABILITIES_REGISTRAR_INTERFACE_H

#include <AVSCommon/SDKInterfaces/Endpoints/EndpointCapabilitiesRegistrarInterface.h>

namespace aace {
namespace test {
namespace unit {
namespace avs {

class MockEndpointCapabilitiesRegistrarInterface
        : public alexaClientSDK::endpoints::EndpointBuilder::EndpointCapabilitiesRegistrarInterface {
public:
    MOCK_METHOD2(
        withCapability,
        EndpointCapabilitiesRegistrarInterface&(
            const alexaClientSDK::avsCommon::avs::CapabilityConfiguration& configuration,
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveHandlerInterface> directiveHandler));

    MOCK_METHOD2(
        withCapability,
        EndpointCapabilitiesRegistrarInterface&(
            const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface>&
                configurationInterface,
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveHandlerInterface> directiveHandler));

    MOCK_METHOD1(
        withCapabilityConfiguration,
        EndpointCapabilitiesRegistrarInterface&(
            const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface>&
                configurationInterface));
};

}  // namespace avs
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_UNIT_AVS_MOCK_ENDPOINT_CAPABILITIES_REGISTRAR_INTERFACE_H
