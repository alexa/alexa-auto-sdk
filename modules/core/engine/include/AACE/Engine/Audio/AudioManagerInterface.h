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

#ifndef AACE_ENGINE_AUDIO_AUDIO_MANAGER_INTERFACE_H
#define AACE_ENGINE_AUDIO_AUDIO_MANAGER_INTERFACE_H

#include <AACE/Audio/AudioInputProvider.h>
#include <AACE/Audio/AudioOutputProvider.h>
#include "AudioInputChannelInterface.h"
#include "AudioOutputChannelInterface.h"

namespace aace {
namespace engine {
namespace audio {

class AudioManagerInterface {
public:
    virtual ~AudioManagerInterface() = default;

    using AudioInputType = aace::audio::AudioInputProvider::AudioInputType;
    using AudioOutputType = aace::audio::AudioOutputProvider::AudioOutputType;

    virtual std::shared_ptr<AudioInputChannelInterface> openAudioInputChannel(
        const std::string& name,
        AudioInputType audioInputType) = 0;
    virtual std::shared_ptr<AudioOutputChannelInterface> openAudioOutputChannel(
        const std::string& name,
        AudioOutputType audioOutputType) = 0;
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_AUDIO_MANAGER_INTERFACE_H
