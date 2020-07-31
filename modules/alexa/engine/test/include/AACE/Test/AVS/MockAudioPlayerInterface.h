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

#ifndef AACE_TEST_UNIT_MOCK_AUDIOPLAYER_INTERFACE_H_
#define AACE_TEST_UNIT_MOCK_AUDIOPLAYER_INTERFACE_H_

#include <AVSCommon/SDKInterfaces/AudioPlayerInterface.h>

namespace aace {
namespace test {
namespace avs {

class MockAudioPlayerInterface : public alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface {
public:
    MOCK_METHOD1(
        addObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> observer));
    MOCK_METHOD1(
        removeObserver,
        void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> observer));
    MOCK_METHOD0(stopPlayback, void());
};

}  // namespace avs
}  // namespace test
}  // namespace aace

#endif  //AACE_TEST_UNIT_MOCK_AUDIOPLAYER_INTERFACE_H_
