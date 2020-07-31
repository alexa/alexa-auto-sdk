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

#include <AACE/Engine/Audio/AudioInputProviderEngineImpl.h>
#include <AACE/Engine/Audio/AudioInputEngineImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>

// String to identify log entries originating from this file.
static const std::string TAG("aace.audio.AudioInputProviderEngineImpl");

namespace aace {
namespace engine {
namespace audio {

AudioInputProviderEngineImpl::AudioInputProviderEngineImpl(
    std::shared_ptr<aace::audio::AudioInputProvider> platformAudioInputProviderInterface) :
        m_platformAudioInputProviderInterface(platformAudioInputProviderInterface) {
}

std::shared_ptr<AudioInputProviderEngineImpl> AudioInputProviderEngineImpl::create(
    std::shared_ptr<aace::audio::AudioInputProvider> platformAudioInputProviderInterface) {
    try {
        ThrowIfNull(platformAudioInputProviderInterface, "invalidAudioInputProviderPlatformInterface");
        return std::shared_ptr<AudioInputProviderEngineImpl>(
            new AudioInputProviderEngineImpl(platformAudioInputProviderInterface));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<AudioInputChannelInterface> AudioInputProviderEngineImpl::openChannel(
    const std::string& name,
    aace::audio::AudioInputProvider::AudioInputType audioInputType) {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        // get the audio input instance from the platform
        auto platformAudioInput = m_platformAudioInputProviderInterface->openChannel(name, audioInputType);
        ThrowIfNull(platformAudioInput, "invalidPlatformAudioInput");

        // see if the instance already exists in our map
        auto it = m_audioInputMap.find(platformAudioInput);
        ReturnIf(it != m_audioInputMap.end(), it->second);

        // create audio input channel engine impl
        auto audioInputChannel = AudioInputEngineImpl::create(platformAudioInput);
        ThrowIfNull(audioInputChannel, "invalidAudioInputChannel");

        // add the audio input channel to the map
        m_audioInputMap[platformAudioInput] = audioInputChannel;

        return audioInputChannel;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "openChannel").d("reason", ex.what()));
        return nullptr;
    }
}

bool AudioInputProviderEngineImpl::doShutdown() {
    for (auto const& audioInput : m_audioInputMap) {
        audioInput.first->setEngineInterface(nullptr);
        audioInput.second->doShutdown();
    }
    m_audioInputMap.clear();

    return true;
}

}  // namespace audio
}  // namespace engine
}  // namespace aace
