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

#include <typeinfo>

#include <AACE/Engine/Audio/AudioEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace audio {

// String to identify log entries originating from this file.
static const std::string TAG("aace.audio.AudioEngineService");

// register the service
REGISTER_SERVICE(AudioEngineService)

AudioEngineService::AudioEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool AudioEngineService::initialize() {
    try {
        ThrowIfNot(
            registerServiceInterface<AudioManagerInterface>(
                std::dynamic_pointer_cast<AudioManagerInterface>(shared_from_this())),
            "registerAudioManagerInterfaceFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

bool AudioEngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::audio::AudioInputProvider>(platformInterface), true);
        ReturnIf(registerPlatformInterfaceType<aace::audio::AudioOutputProvider>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool AudioEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::audio::AudioInputProvider> audioInputProvider) {
    try {
        ThrowIfNotNull(m_audioInputProvideEngineImpl, "platformInterfaceAlreadyRegistered");
        m_audioInputProvideEngineImpl = AudioInputProviderEngineImpl::create(audioInputProvider);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<AudioInputProvider>").d("reason", ex.what()));
        return false;
    }
}

bool AudioEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::audio::AudioOutputProvider> audioOutputProvider) {
    try {
        ThrowIfNotNull(m_audioOutputProvideEngineImpl, "platformInterfaceAlreadyRegistered");
        m_audioOutputProvideEngineImpl = AudioOutputProviderEngineImpl::create(audioOutputProvider);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<AudioOutputProvider>").d("reason", ex.what()));
        return false;
    }
}

//
// AudioManagerInterface Implementation
//

std::shared_ptr<AudioInputChannelInterface> AudioEngineService::openAudioInputChannel(
    const std::string& name,
    AudioInputType audioInputType) {
    try {
        ThrowIfNull(m_audioInputProvideEngineImpl, "invalidAudioInputProvider");

        // attempt to open an audio input channel
        auto channel = m_audioInputProvideEngineImpl->openChannel(name, audioInputType);
        ThrowIfNull(channel, "openAudioInputChannelFailed");

        return channel;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "openAudioInputChannel").d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<AudioOutputChannelInterface> AudioEngineService::openAudioOutputChannel(
    const std::string& name,
    AudioOutputType audioOutputType) {
    try {
        ThrowIfNull(m_audioOutputProvideEngineImpl, "invalidAudioOutputProvider");

        // attempt to open an audio output channel
        auto channel = m_audioOutputProvideEngineImpl->openChannel(name, audioOutputType);
        ThrowIfNull(channel, "openAudioOutputChannelFailed");

        return channel;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "openAudioOutputChannel").d("reason", ex.what()));
        return nullptr;
    }
}

bool AudioEngineService::shutdown() {
    if (m_audioInputProvideEngineImpl != nullptr) {
        m_audioInputProvideEngineImpl->doShutdown();
        m_audioInputProvideEngineImpl.reset();
    }
    if (m_audioOutputProvideEngineImpl != nullptr) {
        m_audioOutputProvideEngineImpl->doShutdown();
        m_audioOutputProvideEngineImpl.reset();
    }
    return true;
}

}  // namespace audio
}  // namespace engine
}  // namespace aace
