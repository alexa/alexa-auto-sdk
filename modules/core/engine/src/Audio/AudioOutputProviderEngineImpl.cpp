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

#include <AACE/Engine/Audio/AudioOutputProviderEngineImpl.h>
#include <AACE/Engine/Audio/AudioOutputEngineImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>

// String to identify log entries originating from this file.
static const std::string TAG("aace.audio.AudioOutputProviderEngineImpl");

namespace aace {
namespace engine {
namespace audio {

AudioOutputProviderEngineImpl::AudioOutputProviderEngineImpl(
    std::shared_ptr<aace::audio::AudioOutputProvider> platformAudioOutputProviderInterface) :
        m_platformAudioOutputProviderInterface(platformAudioOutputProviderInterface) {
}

std::shared_ptr<AudioOutputProviderEngineImpl> AudioOutputProviderEngineImpl::create(
    std::shared_ptr<aace::audio::AudioOutputProvider> platformAudioOutputProviderInterface) {
    try {
        ThrowIfNull(platformAudioOutputProviderInterface, "invalidAudioOutputProviderPlatformInterface");
        return std::shared_ptr<AudioOutputProviderEngineImpl>(
            new AudioOutputProviderEngineImpl(platformAudioOutputProviderInterface));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<AudioOutputChannelInterface> AudioOutputProviderEngineImpl::openChannel(
    const std::string& name,
    aace::audio::AudioOutputProvider::AudioOutputType audioOutputType) {
    try {
        // get the audio input instance from the platform
        auto platformAudioOutput = m_platformAudioOutputProviderInterface->openChannel(name, audioOutputType);
        ThrowIfNull(platformAudioOutput, "invalidPlatformAudioOutput");

        // create audio input channel engine impl
        auto audioOutputChannel = AudioOutputEngineImpl::create(platformAudioOutput);
        ThrowIfNull(audioOutputChannel, "invalidAudioOutputChannel");
        m_audioOutputMap[platformAudioOutput] = audioOutputChannel;

        return audioOutputChannel;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "openChannel").d("reason", ex.what()));
        return nullptr;
    }
}

bool AudioOutputProviderEngineImpl::doShutdown() {
    for (auto const& audioOutput : m_audioOutputMap) {
        audioOutput.first->setEngineInterface(nullptr);
    }
    m_audioOutputMap.clear();
    return true;
}

}  // namespace audio
}  // namespace engine
}  // namespace aace
