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

#ifndef AACE_TEST_AVS_MOCK_MESSAGE_ROUTER_H
#define AACE_TEST_AVS_MOCK_MESSAGE_ROUTER_H

#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>
#include <ACL/Transport/MessageRouterInterface.h>

namespace aace {
namespace test {
namespace avs {

class MockMessageRouter : public alexaClientSDK::acl::MessageRouterInterface {
public:
    MockMessageRouter() : alexaClientSDK::acl::MessageRouterInterface{"MockMessageRouter"} {
    }

    MOCK_METHOD0(enable, void());
    MOCK_METHOD0(disable, void());
    MOCK_METHOD0(doShutdown, void());
    MOCK_METHOD0(getConnectionStatus, alexaClientSDK::acl::MessageRouterInterface::ConnectionStatus());
    // TODO: Revert this to use send().
    MOCK_METHOD1(sendMessage, void(std::shared_ptr<alexaClientSDK::avsCommon::avs::MessageRequest> request));
    MOCK_METHOD1(setAVSEndpoint, void(const std::string& avsEndpoint));
    MOCK_METHOD1(setObserver, void(std::shared_ptr<alexaClientSDK::acl::MessageRouterObserverInterface> observer));
    MOCK_METHOD1(setAVSGateway, void(const std::string& avsGateway));
    MOCK_METHOD0(getAVSGateway, std::string());
    MOCK_METHOD0(onWakeConnectionRetry, void());
    MOCK_METHOD0(onWakeVerifyConnectivity, void());
};

}  // namespace avs
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_AVS_MOCK_MESSAGE_ROUTER_H
