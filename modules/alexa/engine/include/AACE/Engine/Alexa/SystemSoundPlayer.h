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

#ifndef AACE_ENGINE_ALEXA_SYSTEM_SOUND_PLAYER_H
#define AACE_ENGINE_ALEXA_SYSTEM_SOUND_PLAYER_H

#include <future>
#include <memory>

#include <AVSCommon/SDKInterfaces/Audio/SystemSoundAudioFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/SystemSoundPlayerInterface.h>

#include <AACE/Engine/Audio/AudioManagerInterface.h>
#include <AACE/Audio/AudioEngineInterfaces.h>
#include <AACE/Audio/AudioFormat.h>

namespace aace {
namespace engine {
namespace alexa {

class SystemSoundPlayer
        : public aace::audio::AudioOutputEngineInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface
        , public std::enable_shared_from_this<SystemSoundPlayer> {
private:
    SystemSoundPlayer() = default;

    bool initialize(
        std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::SystemSoundAudioFactoryInterface>
            audioFactory);

    std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> getAudioChannel();

public:
    static std::shared_ptr<SystemSoundPlayer> create(
        std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::SystemSoundAudioFactoryInterface>
            audioFactory);

    // aace::audio::AudioOutputEngineInterface
    void onMediaStateChanged(MediaState state) override;
    void onMediaError(MediaError error, const std::string& description) override;

    // alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface
    std::shared_future<bool> playTone(Tone tone) override;

private:
    std::weak_ptr<aace::engine::audio::AudioManagerInterface> m_audioManager;
    std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> m_audioOutputChannel;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::SystemSoundAudioFactoryInterface> m_audioFactory;

    std::shared_future<bool> m_sharedFuture;
    std::promise<bool> m_playTonePromise;
    std::mutex m_mutex;
};

//
// SystemSoundAudioStream
//

class SystemSoundAudioStream : public aace::audio::AudioStream {
private:
    SystemSoundAudioStream(
        std::shared_ptr<std::istream> stream,
        alexaClientSDK::avsCommon::utils::MediaType mediaType,
        alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface::Tone tone);

public:
    static std::shared_ptr<SystemSoundAudioStream> create(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::SystemSoundAudioFactoryInterface> audioFactory,
        alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface::Tone tone);

    // aace::audio::AudioStream
    ssize_t read(char* data, const size_t size) override;
    bool isClosed() override;
    std::vector<aace::audio::AudioStreamProperty> getProperties() override;

private:
    std::shared_ptr<std::istream> m_stream;
    alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface::Tone m_tone;
    bool m_closed;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif
