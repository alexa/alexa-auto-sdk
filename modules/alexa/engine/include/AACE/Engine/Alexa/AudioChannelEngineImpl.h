/*
 * Copyright 2017-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_AUDIO_CHANNEL_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_AUDIO_CHANNEL_ENGINE_IMPL_H

#include <istream>
#include <set>
#include <atomic>

#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerInterface.h>
#include <AVSCommon/Utils/MediaPlayer/MediaPlayerInterface.h>
#include <AVSCommon/Utils/MediaPlayer/MediaPlayerObserverInterface.h>
#include "AVSCommon/Utils/MediaPlayer/SourceConfig.h"
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include <AACE/Alexa/AlexaEngineInterfaces.h>
#include <AACE/Engine/Audio/AudioOutputChannelInterface.h>
#include <AACE/Engine/Audio/IStreamAudioStream.h>

#include "DuckingInterface.h"

namespace aace {
namespace engine {
namespace alexa {

class AudioChannelEngineImpl
        : public aace::audio::AudioOutputEngineInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
        , public alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<AudioChannelEngineImpl>
        , public DuckingInterface {
private:
    using MediaState = aace::audio::AudioOutputEngineInterface::MediaState;
    using MediaError = aace::audio::AudioOutputEngineInterface::MediaError;
    using MixingBehavior = alexaClientSDK::avsCommon::sdkInterfaces::audio::MixingBehavior;

public:
    AudioChannelEngineImpl(
        alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type channelVolumeType,
        std::string name = "");

    virtual bool initializeAudioChannel(
        std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> audioOutputChannel,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate = nullptr);

    virtual void doShutdown() override;

    virtual ~AudioChannelEngineImpl() = default;

    int64_t getMediaPosition();
    int64_t getMediaDuration();

    //
    // aace::audio::AudioOutputEngineInterface
    //
    void onMediaStateChanged(MediaState state) override;
    void onMediaError(MediaError error, const std::string& description) override;
    void onAudioFocusEvent(FocusAction action) override;

    //
    // alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
    //
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
        const alexaClientSDK::avsCommon::utils::AudioFormat* format,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config) override;
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
        std::chrono::milliseconds offsetAdjustment,
        const alexaClientSDK::avsCommon::utils::AudioFormat* format,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config) override;
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
        std::shared_ptr<std::istream> stream,
        bool repeat,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
        alexaClientSDK::avsCommon::utils::MediaType format) override;
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
        const std::string& url,
        std::chrono::milliseconds offset,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
        bool repeat,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::PlaybackContext& playbackContext) override;
    bool play(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
    bool stop(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
    bool pause(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
    bool resume(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
    std::chrono::milliseconds getOffset(
        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
    uint64_t getNumBytesBuffered() override;
    alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState>
    getMediaPlayerState(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;
    void addObserver(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> observer) override;
    void removeObserver(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> observer) override;

    //
    // alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface
    //
    bool setVolume(int8_t volume) override;
    bool setMute(bool mute) override;
    bool getSpeakerSettings(
        alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings) override;

    //
    // alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
    //
    void onAuthStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State state,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) override;

    //
    // aace::engine::alexa::DuckingInterface
    //
    bool startDucking() override;
    bool stopDucking() override;

protected:
    using SourceId = alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId;

    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId nextId() {
        return ++s_nextId;
    }

    bool validateSource(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id);

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface> getChannelVolumeInterface();

private:
    enum class PendingEventState { NONE, PLAYBACK_STARTED, PLAYBACK_PAUSED, PLAYBACK_RESUMED, PLAYBACK_STOPPED };

    enum class MediaStateChangeInitiator { NONE, PLAY, PAUSE, RESUME, STOP };

    enum class DuckingStates { NONE, DUCKED_BY_ALEXA, DUCKED_BY_PLATFORM };

    void sendPendingEvent();
    void sendEvent(PendingEventState state);
    void resetSource();
    void execDuckingStarted();
    void execDuckingStopped();
    void setMediaStateChangeInitiator(MediaStateChangeInitiator initiator);

    //
    // MediaPlayerEngineInterface executor methods
    //
    void executeMediaStateChanged(SourceId id, MediaState state);
    void executeMediaError(SourceId id, MediaError error, const std::string& description);
    void executePlaybackStarted(SourceId id);
    void executePlaybackFinished(SourceId id);
    void executePlaybackPaused(SourceId id);
    void executePlaybackResumed(SourceId id);
    void executePlaybackStopped(SourceId id);
    void executePlaybackError(SourceId id, MediaError error, const std::string& description);
    void executeBufferUnderrun(SourceId id);
    void executeBufferRefilled(SourceId id);

    friend std::ostream& operator<<(std::ostream& stream, const PendingEventState& state);
    friend std::ostream& operator<<(std::ostream& stream, const DuckingStates& state);

    //
    // MediaPlayerInterface implementation in executor
    //
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId execSetSource(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
        const alexaClientSDK::avsCommon::utils::AudioFormat* format,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config);
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId execSetSource(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
        std::chrono::milliseconds offsetAdjustment,
        const alexaClientSDK::avsCommon::utils::AudioFormat* format,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config);
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId execSetSource(
        std::shared_ptr<std::istream> stream,
        bool repeat,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
        alexaClientSDK::avsCommon::utils::MediaType format);
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId execSetSource(
        const std::string& url,
        std::chrono::milliseconds offset,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
        bool repeat,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::PlaybackContext& playbackContext);
    bool execPlay(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id);
    bool execStop(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id);
    bool execPause(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id);
    bool execResume(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id);
    std::chrono::milliseconds execGetOffset(
        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id);
    uint64_t execGetNumBytesBuffered();
    alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState>
    execGetMediaPlayerState(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id);

    bool execStartDucking();
    bool execStopDucking();

private:
    std::string m_name;
    std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> m_audioOutputChannel;

    alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type m_channelVolumeType;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface> m_channelVolumeInterface;

    std::weak_ptr<class AttachmentReaderAudioStream> m_attachmentReader;

    // access to m_mediaPlayerObservers is serialized by @c m_callbackExecutor
    using MediaPlayerObserverInterface = alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface;
    std::set<std::weak_ptr<MediaPlayerObserverInterface>, std::owner_less<std::weak_ptr<MediaPlayerObserverInterface>>>
        m_mediaPlayerObservers;

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> m_speakerManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> m_authDelegate;

    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId m_currentId;
    std::string m_url;
    std::chrono::milliseconds m_savedOffset;
    bool m_muted;
    int8_t m_volume;

    PendingEventState m_pendingEventState;
    MediaState m_currentMediaState;
    MediaStateChangeInitiator m_mediaStateChangeInitiator;

    // executor used to synchronize external requests and notifications
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
    // executor used to send asynchronous events back to observer
    alexaClientSDK::avsCommon::utils::threading::Executor m_callbackExecutor;

    // global counter for media source id
    static SourceId s_nextId;

    //variable for storing the mixability of the current stream
    bool m_mayDuck;

    DuckingStates m_duckingState;
};

inline std::ostream& operator<<(std::ostream& stream, const AudioChannelEngineImpl::PendingEventState& state) {
    switch (state) {
        case AudioChannelEngineImpl::PendingEventState::NONE:
            stream << "NONE";
            break;
        case AudioChannelEngineImpl::PendingEventState::PLAYBACK_STARTED:
            stream << "PLAYBACK_STARTED";
            break;
        case AudioChannelEngineImpl::PendingEventState::PLAYBACK_PAUSED:
            stream << "PLAYBACK_PAUSED";
            break;
        case AudioChannelEngineImpl::PendingEventState::PLAYBACK_RESUMED:
            stream << "PLAYBACK_RESUMED";
            break;
        case AudioChannelEngineImpl::PendingEventState::PLAYBACK_STOPPED:
            stream << "PLAYBACK_STOPPED";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const AudioChannelEngineImpl::DuckingStates& state) {
    switch (state) {
        case AudioChannelEngineImpl::DuckingStates::DUCKED_BY_ALEXA:
            stream << "DUCKED_BY_ALEXA";
            break;
        case AudioChannelEngineImpl::DuckingStates::DUCKED_BY_PLATFORM:
            stream << "DUCKED_BY_PLATFORM";
            break;
        default:
            stream << "NONE";
            break;
    }
    return stream;
}

//
// AttachmentReaderAudioStream
//

class AttachmentReaderAudioStream : public aace::audio::AudioStream {
private:
    AttachmentReaderAudioStream(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
        const AudioFormat& audioFormat = AudioFormat::UNKNOWN);

public:
    static std::shared_ptr<AttachmentReaderAudioStream> create(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
        const alexaClientSDK::avsCommon::utils::AudioFormat* format);

    // aace::audio::AudioStream
    ssize_t read(char* data, const size_t size) override;
    bool isClosed() override;
    AudioFormat getAudioFormat() override;

    void close();

private:
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> m_attachmentReader;
    alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus m_status;
    std::atomic<bool> m_closed;
    AudioFormat m_audioFormat;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_AUDIO_CHANNEL_ENGINE_IMPL_H
