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

#ifndef AACE_TEST_UNIT_AVS_MOCK_PLAYBACK_ROUTER_INTERFACE_H
#define AACE_TEST_UNIT_AVS_MOCK_PLAYBACK_ROUTER_INTERFACE_H

#include <AVSCommon/SDKInterfaces/PlaybackRouterInterface.h>

namespace aace {
namespace test {
namespace unit {
namespace avs {

// Mock alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface

class MockPlaybackRouterInterface : public alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface {
public:
    MOCK_METHOD1(buttonPressed, void(alexaClientSDK::avsCommon::avs::PlaybackButton button));
    MOCK_METHOD2(togglePressed, void(alexaClientSDK::avsCommon::avs::PlaybackToggle toggle, bool action));
    MOCK_METHOD2(
        setHandler,
        void(
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackHandlerInterface> handler,
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocalPlaybackHandlerInterface> localHandler));
    MOCK_METHOD0(switchToDefaultHandler, void());
    MOCK_METHOD1(
        useDefaultHandlerWith,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocalPlaybackHandlerInterface> localHandler));
    MOCK_METHOD1(
        localOperation,
        bool(alexaClientSDK::avsCommon::sdkInterfaces::LocalPlaybackHandlerInterface::PlaybackOperation op));
    MOCK_METHOD2(localSeekTo, bool(std::chrono::milliseconds location, bool fromStart));
};

}  // namespace avs
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_UNIT_AVS_MOCK_PLAYBACK_ROUTER_INTERFACE_H
