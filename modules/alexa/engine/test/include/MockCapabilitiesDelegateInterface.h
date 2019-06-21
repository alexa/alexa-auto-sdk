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

#ifndef AACE_TEST_UNIT_MOCK_CAPABILITIESDELEGATE_INTERFACE_H_
#define AACE_TEST_UNIT_MOCK_CAPABILITIESDELEGATE_INTERFACE_H_

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon;

class MockCapabilitiesDelegateInterface : public alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface {
public:
    MOCK_METHOD1(registerCapability,
        bool(const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface>& capabilitiesProvider));
    MOCK_METHOD0(publishCapabilities,
        CapabilitiesPublishReturnCode());
    MOCK_METHOD0(publishCapabilitiesAsyncWithRetries,
        void());
    MOCK_METHOD0(invalidateCapabilities,
        void());
    MOCK_METHOD1(addCapabilitiesObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface> observer));
    MOCK_METHOD1(removeCapabilitiesObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface> observer));
    MOCK_METHOD2(onAuthStateChange,
        void(alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State newState, 
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error newError));
    MOCK_METHOD0(doShutdown,
        void());
};


}
}
}
#endif //AACE_TEST_UNIT_MOCK_CAPABILITIESDELEGATE_INTERFACE_H_