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

#ifndef AACE_ENGINE_AUDIO_AUDIO_ENGINE_SERVICE_H
#define AACE_ENGINE_AUDIO_AUDIO_ENGINE_SERVICE_H

#include <AACE/Engine/Core/EngineService.h>
#include <AACE/Audio/AudioInputProvider.h>
#include <AACE/Audio/AudioOutputProvider.h>

#include "AudioManagerInterface.h"
#include "AudioInputProviderEngineImpl.h"
#include "AudioOutputProviderEngineImpl.h"

namespace aace {
namespace engine {
namespace audio {

class AudioEngineService
        : public aace::engine::core::EngineService
        , public AudioManagerInterface
        , public std::enable_shared_from_this<AudioEngineService> {
public:
    DESCRIBE("aace.audio", VERSION("1.0"))

private:
    AudioEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~AudioEngineService() = default;

    // AudioManagerInterface
    std::shared_ptr<AudioInputChannelInterface> openAudioInputChannel(
        const std::string& name,
        AudioInputType audioInputType) override;
    std::shared_ptr<AudioOutputChannelInterface> openAudioOutputChannel(
        const std::string& name,
        AudioOutputType audioOutputType) override;

protected:
    bool initialize() override;
    bool shutdown() override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::audio::AudioInputProvider> audioInputProvider);
    bool registerPlatformInterfaceType(std::shared_ptr<aace::audio::AudioOutputProvider> audioOutputProvider);

private:
    std::shared_ptr<AudioInputProviderEngineImpl> m_audioInputProvideEngineImpl;
    std::shared_ptr<AudioOutputProviderEngineImpl> m_audioOutputProvideEngineImpl;
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_AUDIO_ENGINE_SERVICE_H
