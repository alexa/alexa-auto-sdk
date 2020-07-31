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

#ifndef AACE_ENGINE_TEST_AUDIO_MOCK_AUDIO_OUTPUT_CHANNEL_INTERFACE_H
#define AACE_ENGINE_TEST_AUDIO_MOCK_AUDIO_OUTPUT_CHANNEL_INTERFACE_H

#include "AACE/Engine/Audio/AudioOutputChannelInterface.h"
#include <gtest/gtest.h>

namespace aace {
namespace test {
namespace audio {

class MockAudioOutputChannelInterface : public aace::engine::audio::AudioOutputChannelInterface {
public:
    MOCK_METHOD2(prepare, bool(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating));
    MOCK_METHOD2(prepare, bool(const std::string& url, bool repeating));
    MOCK_METHOD0(play, bool());
    MOCK_METHOD0(stop, bool());
    MOCK_METHOD0(pause, bool());
    MOCK_METHOD0(resume, bool());
    MOCK_METHOD0(getPosition, int64_t());
    MOCK_METHOD1(setPosition, bool(int64_t position));
    MOCK_METHOD0(getDuration, int64_t());
    MOCK_METHOD0(getNumBytesBuffered, int64_t());
    MOCK_METHOD1(volumeChanged, bool(float volume));
    MOCK_METHOD1(mutedStateChanged, bool(MutedState state));
    MOCK_METHOD1(
        setEngineInterface,
        void(std::shared_ptr<aace::audio::AudioOutputEngineInterface> audioOutputEngineInterface));
};

}  // namespace audio
}  // namespace test
}  // namespace aace

#endif  // AACE_ENGINE_TEST_AUDIO_MOCK_AUDIO_OUTPUT_CHANNEL_INTERFACE_H
