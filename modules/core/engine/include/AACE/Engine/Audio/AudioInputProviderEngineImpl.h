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

#ifndef AACE_ENGINE_AUDIO_AUDIO_INPUT_PROVIDER_ENGINE_IMPL_H
#define AACE_ENGINE_AUDIO_AUDIO_INPUT_PROVIDER_ENGINE_IMPL_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include <AACE/Audio/AudioInputProvider.h>

#include "AudioInputChannelInterface.h"

namespace aace {
namespace engine {
namespace audio {

class AudioInputProviderEngineImpl {
private:
    AudioInputProviderEngineImpl(std::shared_ptr<aace::audio::AudioInputProvider> platformAudioInputProviderInterface);

public:
    static std::shared_ptr<AudioInputProviderEngineImpl> create(
        std::shared_ptr<aace::audio::AudioInputProvider> platformAudioInputProviderInterface);
    std::shared_ptr<AudioInputChannelInterface> openChannel(
        const std::string& name,
        aace::audio::AudioInputProvider::AudioInputType audioInputType);
    bool doShutdown();

private:
    std::shared_ptr<aace::audio::AudioInputProvider> m_platformAudioInputProviderInterface;
    std::unordered_map<std::shared_ptr<aace::audio::AudioInput>, std::shared_ptr<AudioInputChannelInterface>>
        m_audioInputMap;

    std::mutex m_mutex;
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_AUDIO_INPUT_PROVIDER_ENGINE_IMPL_H
