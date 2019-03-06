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

#ifndef AACE_TEST_UNIT_MOCK_MESSAGE_ROUTER_H_
#define AACE_TEST_UNIT_MOCK_MESSAGE_ROUTER_H_

#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon;

/**
 * This class allows us to test MessageRouter interaction.
 */
class MockMessageRouter : public alexaClientSDK::acl::MessageRouterInterface {
public:
    MockMessageRouter() : alexaClientSDK::acl::MessageRouterInterface{"MockMessageRouter"} {
    }

    MOCK_METHOD0(enable, void());
    MOCK_METHOD0(disable, void());
    MOCK_METHOD0(doShutdown, void());
    MOCK_METHOD0(getConnectionStatus, alexaClientSDK::acl::MessageRouterInterface::ConnectionStatus());
    // TODO: ACSDK-421: Revert this to use send().
    MOCK_METHOD1(sendMessage, void(std::shared_ptr<avsCommon::avs::MessageRequest> request));
    MOCK_METHOD1(setAVSEndpoint, void(const std::string& avsEndpoint));
    MOCK_METHOD1(setObserver, void(std::shared_ptr<alexaClientSDK::acl::MessageRouterObserverInterface> observer));
};

}
}
}
#endif //AACE_TEST_UNIT_MOCK_MESSAGE_ROUTER_H_