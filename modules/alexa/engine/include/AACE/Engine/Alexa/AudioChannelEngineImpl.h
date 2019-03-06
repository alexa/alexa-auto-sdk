/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AVSCommon/Utils/MediaPlayer/MediaPlayerInterface.h>
#include <AVSCommon/Utils/MediaPlayer/MediaPlayerObserverInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerInterface.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/AudioChannel.h"
#include "AACE/Alexa/MediaPlayer.h"
#include "AACE/Alexa/Speaker.h"

namespace aace {
namespace engine {
namespace alexa {

class AudioChannelEngineImpl :
    public aace::alexa::MediaPlayerEngineInterface,
    public aace::alexa::SpeakerEngineInterface,
    public alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface,
    public alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<AudioChannelEngineImpl> {

public:
    AudioChannelEngineImpl( std::shared_ptr<aace::alexa::AudioChannel> audioChannelPlatformInterface, const std::string& name );

    virtual bool initializeAudioChannel( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager );
    
    virtual void doShutdown() override;
    
    virtual ~AudioChannelEngineImpl() = default;

    //
    // aace::engine::MediaPlayerEngineInterface
    //
    void onMediaStateChanged( MediaState state ) override;
    void onMediaError( MediaError error, const std::string& description = "" ) override;
    ssize_t read( char* data, const size_t size ) override;
    
    bool isRepeating() override;
    bool isClosed() override;

    //
    // aace::engine::SpeakerEngineInterface
    //
    void onLocalVolumeSet( int8_t volume ) override;
    void onLocalMuteSet( bool mute ) override;

    //
    // alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
    //
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource( std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader, const alexaClientSDK::avsCommon::utils::AudioFormat* format = nullptr ) override;
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource( std::shared_ptr<std::istream> stream, bool repeat ) override;
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource( const std::string& url, std::chrono::milliseconds offset ) override;
    bool play( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
    bool stop( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
    bool pause( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
    bool resume( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
    std::chrono::milliseconds getOffset( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
    uint64_t getNumBytesBuffered() override;
    void setObserver( std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> observer ) override;

    //
    // alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface
    //
    bool setVolume( int8_t volume ) override;
    bool adjustVolume( int8_t delta ) override;
    bool setMute( bool mute ) override;
    bool getSpeakerSettings( alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings ) override;
    alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type getSpeakerType() override;
    
protected:
    using SourceId = alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId;

    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> getObserver() {
        return m_observer;
    }
    
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId nextId() {
        return ++s_nextId;
    }
    
    bool validateSource( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id );
    
    virtual void handlePrePlaybackStarted( SourceId id );
    virtual void handlePostPlaybackStarted( SourceId id );
    virtual void handlePrePlaybackFinished( SourceId id );
    virtual void handlePostPlaybackFinished( SourceId id );

private:
    enum class PendingEventState {
        NONE, PLAYBACK_STARTED, PLAYBACK_PAUSED, PLAYBACK_RESUMED, PLAYBACK_STOPPED
    };

    enum class MediaStateChangeInitiator {
        NONE, PLAY, PAUSE, RESUME, STOP
    };
    
    void sendPendingEvent();
    void sendEvent( PendingEventState state );
    void resetSource();
    
    //
    // MediaPlayerEngineInterface executor methods
    //
    void executeMediaStateChanged( SourceId id, MediaState state );
    void executeMediaError( SourceId id, MediaError error, const std::string& description );
    void executePlaybackStarted( SourceId id );
    void executePlaybackFinished( SourceId id );
    void executePlaybackPaused( SourceId id );
    void executePlaybackResumed( SourceId id );
    void executePlaybackStopped( SourceId id );
    void executePlaybackError( SourceId id, MediaError error, const std::string& description );
    void executeBufferUnderrun( SourceId id );
    void executeBufferRefilled( SourceId id );

    friend std::ostream& operator<<(std::ostream& stream, const PendingEventState& state);

private:
    std::shared_ptr<aace::alexa::AudioChannel> m_audioChannelPlatformInterface;
    std::shared_ptr<aace::alexa::MediaPlayer> m_mediaPlayerPlatformInterface;
    std::shared_ptr<aace::alexa::Speaker> m_speakerPlatformInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> m_observer;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> m_attachmentReader;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> m_speakerManager;
    
    alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus m_status;
    std::shared_ptr<std::istream> m_stream;
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId m_currentId;
    bool m_repeat;
    bool m_closed;
    std::string m_url;
    std::chrono::milliseconds m_savedOffset;
    std::string m_name;
    
    PendingEventState m_pendingEventState;
    MediaState m_currentMediaState;
    MediaStateChangeInitiator m_mediaStateChangeInitiator;

    // executor used to send asynchronous events back to observer
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    // global counter for media source id
    static SourceId s_nextId;

    // mutex for blocking setSource calls
    std::mutex m_mutex;

    // wait condition
    std::condition_variable m_trigger;    
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

} // aace::engine::alexa
} // aace::engine
} // aace

#endif // AACE_ENGINE_ALEXA_AUDIO_CHANNEL_ENGINE_IMPL_H

