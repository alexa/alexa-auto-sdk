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

#include <AACE/Engine/Alexa/SystemSoundPlayer.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.SystemSoundPlayer");

std::shared_ptr<SystemSoundPlayer> SystemSoundPlayer::create(
    std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::SystemSoundAudioFactoryInterface> audioFactory) {
    try {
        ThrowIfNull(audioManager, "invalidAudioManager");
        ThrowIfNull(audioFactory, "invalidSystemSoundAudioFactory");

        auto systemSoundPlayer = std::shared_ptr<SystemSoundPlayer>(new SystemSoundPlayer());

        ThrowIfNot(systemSoundPlayer->initialize(audioManager, audioFactory), "initializeSystemSoundPlayerFailed");

        return systemSoundPlayer;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool SystemSoundPlayer::initialize(
    std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::SystemSoundAudioFactoryInterface> audioFactory) {
    try {
        m_audioManager = audioManager;
        m_audioFactory = audioFactory;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> SystemSoundPlayer::getAudioChannel() {
    try {
        // open the EARCON audio channel if it hasn't already been opened
        if (m_audioOutputChannel == nullptr) {
            auto audioManager = m_audioManager.lock();
            ThrowIfNot(audioManager, "invalidAudioManagerReference");

            m_audioOutputChannel = audioManager->openAudioOutputChannel(
                "SystemSoundPlayer", aace::audio::AudioOutputProvider::AudioOutputType::EARCON);
            ThrowIfNull(m_audioOutputChannel, "openAudioOutputChannelFailed");

            m_audioOutputChannel->setEngineInterface(shared_from_this());
        }

        return m_audioOutputChannel;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

//
// aace::audio::AudioOutputEngineInterface
//

void SystemSoundPlayer::onMediaStateChanged(MediaState state) {
    if (state == MediaState::STOPPED) {
        m_playTonePromise.set_value(true);
        m_playTonePromise = std::promise<bool>();
        m_sharedFuture = std::shared_future<bool>();
    }
}

void SystemSoundPlayer::onMediaError(MediaError error, const std::string& description) {
    AACE_ERROR(LX(TAG).d("error", error).d("description", description));
    m_playTonePromise.set_value(false);
    m_playTonePromise = std::promise<bool>();
    m_sharedFuture = std::shared_future<bool>();
}

//
// alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface
//

std::shared_future<bool> SystemSoundPlayer::playTone(Tone tone) {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        ThrowIf(m_sharedFuture.valid(), "toneAlreadyPlaying");

        auto audioChannel = getAudioChannel();
        ThrowIfNull(audioChannel, "invalidAudioChannel");

        // create the audio stream
        auto stream = SystemSoundAudioStream::create(m_audioFactory, tone);
        ThrowIfNull(stream, "invalidAudioStream");

        // prepare the sound to play
        ThrowIfNot(audioChannel->prepare(stream, false), "audioOutputChannelPrepareFailed");
        ThrowIfNot(audioChannel->play(), "audioOutputChannelPlayFailed");

        m_sharedFuture = m_playTonePromise.get_future();

        return m_sharedFuture;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        auto errPromise = std::promise<bool>();
        errPromise.set_value(false);
        return errPromise.get_future();
    }
}

//
// SystemSoundAudioStream
//

SystemSoundAudioStream::SystemSoundAudioStream(
    std::shared_ptr<std::istream> stream,
    alexaClientSDK::avsCommon::utils::MediaType mediaType,
    alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface::Tone tone) :
        m_stream(stream), m_tone(tone), m_closed(false) {
}

std::shared_ptr<SystemSoundAudioStream> SystemSoundAudioStream::create(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::SystemSoundAudioFactoryInterface> audioFactory,
    alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface::Tone tone) {
    std::shared_ptr<SystemSoundAudioStream> stream;

    std::shared_ptr<std::istream> iStream;
    alexaClientSDK::avsCommon::utils::MediaType streamFormat = alexaClientSDK::avsCommon::utils::MediaType::UNKNOWN;

    switch (tone) {
        case alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface::Tone::WAKEWORD_NOTIFICATION:
            std::tie(iStream, streamFormat) = audioFactory->wakeWordNotificationTone()();
            stream = std::shared_ptr<SystemSoundAudioStream>(new SystemSoundAudioStream(
                iStream,
                streamFormat,
                alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface::Tone::WAKEWORD_NOTIFICATION));
            break;

        case alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface::Tone::END_SPEECH:
            std::tie(iStream, streamFormat) = audioFactory->endSpeechTone()();
            stream = std::shared_ptr<SystemSoundAudioStream>(new SystemSoundAudioStream(
                iStream,
                streamFormat,
                alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface::Tone::END_SPEECH));
            break;
    }

    return stream;
}

ssize_t SystemSoundAudioStream::read(char* data, const size_t size) {
    try {
        if (m_stream->eof()) {
            m_closed = true;
            return 0;
        }

        // read the data from the stream
        m_stream->read(data, size);
        ThrowIf(m_stream->bad(), "readFailed");

        // get the number of bytes read
        ssize_t count = m_stream->gcount();

        m_stream->tellg();  // Don't remove otherwise the ReseourceStream used for Alerts/Timers won't work as expected.

        return count;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG + ".SystemSoundAudioStream").d("reason", ex.what()).d("size", size).d("closed", m_closed));
        m_closed = true;
        return 0;
    }
}

bool SystemSoundAudioStream::isClosed() {
    return m_closed;
}

std::vector<aace::audio::AudioStreamProperty> SystemSoundAudioStream::getProperties() {
    return {{"cache-policy", "ALWAYS"},
            {"cache-id", "aace.alexa.SystemSoundPlayer#" + std::to_string(static_cast<int>(m_tone))}};
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
