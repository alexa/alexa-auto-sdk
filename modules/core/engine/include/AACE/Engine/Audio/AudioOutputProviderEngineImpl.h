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

#ifndef AACE_ENGINE_AUDIO_AUDIO_OUTPUT_PROVIDER_ENGINE_IMPL_H
#define AACE_ENGINE_AUDIO_AUDIO_OUTPUT_PROVIDER_ENGINE_IMPL_H

#include <memory>
#include <unordered_map>

#include <AACE/Audio/AudioOutputProvider.h>
#include "AudioOutputChannelInterface.h"

namespace aace {
namespace engine {
namespace audio {

class AudioOutputProviderEngineImpl {
public:
    static std::shared_ptr<AudioOutputProviderEngineImpl> create(
        std::shared_ptr<aace::audio::AudioOutputProvider> platformAudioOutputProviderInterface);

    std::shared_ptr<AudioOutputChannelInterface> openChannel(
        const std::string& name,
        aace::audio::AudioOutputProvider::AudioOutputType audioOutputType);
    bool doShutdown();

private:
    AudioOutputProviderEngineImpl(
        std::shared_ptr<aace::audio::AudioOutputProvider> platformAudioOutputProviderInterface);
    std::unordered_map<std::shared_ptr<aace::audio::AudioOutput>, std::shared_ptr<AudioOutputChannelInterface>>
        m_audioOutputMap;

private:
    std::shared_ptr<aace::audio::AudioOutputProvider> m_platformAudioOutputProviderInterface;
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_AUDIO_OUTPUT_PROVIDER_ENGINE_IMPL_H
