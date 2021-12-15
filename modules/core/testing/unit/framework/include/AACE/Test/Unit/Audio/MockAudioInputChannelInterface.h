/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_UNIT_AUDIO_MOCK_AUDIO_INPUT_CHANNEL_INTERFACE_H
#define AACE_TEST_UNIT_AUDIO_MOCK_AUDIO_INPUT_CHANNEL_INTERFACE_H

#include "AACE/Engine/Audio/AudioOutputChannelInterface.h"
#include <gtest/gtest.h>

namespace aace {
namespace test {
namespace unit {
namespace audio {

class MockAudioInputChannelInterface : public aace::engine::audio::AudioInputChannelInterface {
public:
    MOCK_METHOD1(start, ChannelId(AudioWriteCallback callback));
    MOCK_METHOD1(stop, void(ChannelId id));
    MOCK_METHOD0(doShutdown, void());
};

}  // namespace audio
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_UNIT_AUDIO_MOCK_AUDIO_INPUT_CHANNEL_INTERFACE_H
