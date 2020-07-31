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

#ifndef AACE_ENGINE_AUDIO_AUDIO_OUTPUT_CHANNEL_ENGINE_IMPL_H
#define AACE_ENGINE_AUDIO_AUDIO_OUTPUT_CHANNEL_ENGINE_IMPL_H

#include <memory>

#include <AACE/Audio/AudioOutput.h>
#include "AudioOutputChannelInterface.h"

namespace aace {
namespace engine {
namespace audio {

class AudioOutputEngineImpl
        : public AudioOutputChannelInterface
        , public aace::audio::AudioOutputEngineInterface {
public:
    static std::shared_ptr<AudioOutputEngineImpl> create(std::shared_ptr<aace::audio::AudioOutput> platformAudioOutput);

private:
    AudioOutputEngineImpl(std::shared_ptr<aace::audio::AudioOutput> platformAudioOutput);

public:
    // AudioOutputChannelInterface
    bool prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) override;
    bool prepare(const std::string& url, bool repeating) override;
    bool play() override;
    bool stop() override;
    bool pause() override;
    bool resume() override;
    int64_t getPosition() override;
    bool setPosition(int64_t position) override;
    int64_t getDuration() override;
    int64_t getNumBytesBuffered() override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(MutedState state) override;
    void setEngineInterface(
        std::shared_ptr<aace::audio::AudioOutputEngineInterface> audioOutputEngineInterface) override;

    // aace::audio::AudioOutputEngineInterface
    void onMediaStateChanged(MediaState state) override;
    void onMediaError(MediaError error, const std::string& description = "") override;

private:
    std::shared_ptr<aace::audio::AudioOutput> m_platformAudioOutput;
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_AUDIO_OUTPUT_CHANNEL_ENGINE_IMPL_H
