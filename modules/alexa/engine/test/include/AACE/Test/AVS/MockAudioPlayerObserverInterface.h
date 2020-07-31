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

#ifndef AACE_TEST_UNIT_MOCK_AUDIOPLAYER_OBSERVER_INTERFACE_H_
#define AACE_TEST_UNIT_MOCK_AUDIOPLAYER_OBSERVER_INTERFACE_H_

#include <AVSCommon/SDKInterfaces/AudioPlayerObserverInterface.h>

namespace aace {
namespace test {
namespace avs {

class MockAudioPlayerObserverInterface : public alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface {
public:
    MOCK_METHOD2(
        onPlayerActivityChanged,
        void(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context));
};

}  // namespace avs
}  // namespace test
}  // namespace aace

#endif  //AACE_TEST_UNIT_MOCK_AUDIOPLAYER_OBSERVER_INTERFACE_H_
