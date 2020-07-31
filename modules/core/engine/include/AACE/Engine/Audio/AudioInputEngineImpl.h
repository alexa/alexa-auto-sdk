/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_AUDIO_AUDIO_INPUT_ENGINE_IMPL_H
#define AACE_ENGINE_AUDIO_AUDIO_INPUT_ENGINE_IMPL_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include <AACE/Audio/AudioInput.h>
#include "AudioInputChannelInterface.h"

namespace aace {
namespace engine {
namespace audio {

class AudioInputEngineImpl
        : public aace::audio::AudioInputEngineInterface
        , public AudioInputChannelInterface {
private:
    AudioInputEngineImpl(std::shared_ptr<aace::audio::AudioInput> platformAudioInput);

public:
    static std::shared_ptr<AudioInputEngineImpl> create(std::shared_ptr<aace::audio::AudioInput> platformAudioInput);

    // AudioInputChannelInterface
    ChannelId start(AudioWriteCallback callback) override;
    bool stop(ChannelId id) override;
    void doShutdown() override;

    // AudioInputChannelEngineInterface
    ssize_t write(const int16_t* data, const size_t size) override;

private:
    ChannelId getNextChannelId();

private:
    std::shared_ptr<aace::audio::AudioInput> m_platformAudioInput;
    std::unordered_map<ChannelId, AudioWriteCallback> m_callbackMap;

    ChannelId m_nextChannelId = 1;

    std::mutex m_mutex;          // to serialize operations of AudioInputChannelInterface
    std::mutex m_callbackMutex;  // to guard against potential race conditions caused by callback from another thread
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_AUDIO_INPUT_ENGINE_IMPL_H
