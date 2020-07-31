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

#ifndef AACE_TEST_AVS_MOCK_RENDER_PLAYER_INFO_CARDS_PROVIDER_INTERFACE_H
#define AACE_TEST_AVS_MOCK_RENDER_PLAYER_INFO_CARDS_PROVIDER_INTERFACE_H

#include <AVSCommon/SDKInterfaces/RenderPlayerInfoCardsObserverInterface.h>

namespace aace {
namespace test {
namespace avs {

class MockRenderPlayerInfoCardsProviderInterface
        : public alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface {
public:
    MOCK_METHOD1(
        setObserver,
        bool(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface>
                 observer));
    MOCK_METHOD2(
        onRenderPlayerCardsInfoChanged,
        void(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context));
};

}  // namespace avs
}  // namespace test
}  // namespace aace

#endif  //AACE_TEST_AVS_MOCK_RENDER_PLAYER_INFO_CARDS_PROVIDER_INTERFACE_H
