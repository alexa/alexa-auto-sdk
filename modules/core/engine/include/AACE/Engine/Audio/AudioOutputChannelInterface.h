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

#ifndef AACE_ENGINE_AUDIO_AUDIO_OUTPUT_CHANNEL_INTERFACE_H
#define AACE_ENGINE_AUDIO_AUDIO_OUTPUT_CHANNEL_INTERFACE_H

#include <AACE/Audio/AudioOutput.h>

namespace aace {
namespace engine {
namespace audio {

class AudioOutputChannelInterface {
public:
    virtual ~AudioOutputChannelInterface() = default;

    using MutedState = aace::audio::AudioOutput::MutedState;

    virtual bool prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) = 0;
    virtual bool prepare(const std::string& url, bool repeating) = 0;
    virtual bool play() = 0;
    virtual bool stop() = 0;
    virtual bool pause() = 0;
    virtual bool resume() = 0;
    virtual int64_t getPosition() = 0;
    virtual bool setPosition(int64_t position) = 0;
    virtual int64_t getDuration() = 0;
    virtual int64_t getNumBytesBuffered() = 0;
    virtual bool volumeChanged(float volume) = 0;
    virtual bool mutedStateChanged(MutedState state) = 0;
    virtual void setEngineInterface(
        std::shared_ptr<aace::audio::AudioOutputEngineInterface> audioOutputEngineInterface) = 0;
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_AUDIO_OUTPUT_CHANNEL_INTERFACE_H
