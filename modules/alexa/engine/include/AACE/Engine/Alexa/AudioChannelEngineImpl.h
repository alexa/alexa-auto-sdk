/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/AudioChannel.h"
#include "AACE/Alexa/MediaPlayer.h"
#include "AACE/Alexa/Speaker.h"

namespace aace {
namespace engine {
namespace alexa {

class AudioChannelEngineImpl :
    public aace::alexa::MediaPlayerEngineInterface,
    public alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface,
    public alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface,
    public std::enable_shared_from_this<AudioChannelEngineImpl> {
    
protected:
    AudioChannelEngineImpl( std::shared_ptr<aace::alexa::AudioChannel> audioChannelPlatformInterface );

public:
    virtual ~AudioChannelEngineImpl();

    //
    // aace::engine::MediaPlayerEngineInterface
    //
    void onPlaybackStarted() override;
    void onPlaybackFinished() override;
    void onPlaybackPaused() override;
    void onPlaybackResumed() override;
    void onPlaybackStopped() override;
    void onPlaybackError( const MediaPlayerEngineInterface::ErrorType& type, const std::string& error ) override;
    ssize_t read( char* data, const size_t size ) override;
    bool isRepeating() override;

    //
    // alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
    //
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource( std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader ) override;
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource( std::shared_ptr<std::istream> stream, bool repeat ) override;
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource( const std::string& url, std::chrono::milliseconds offset ) override;
    bool play( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
    bool stop( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
    bool pause( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
    bool resume( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
    std::chrono::milliseconds getOffset( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id ) override;
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
    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> getObserver() {
        return m_observer;
    }
    
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId nextId() {
        return ++s_nextId;
    }
    
    bool validateSource( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id );
    
private:
    enum class EventState {
        NONE, PLAYBACK_STARTED, PLAYBACK_FINISHED, PLAYBACK_PAUSED, PLAYBACK_RESUMED, PLAYBACK_STOPPED, PLAYBACK_ERROR
    };
    

    void setEventState( EventState state );
    bool waitForEventState( const std::chrono::seconds duration = std::chrono::seconds( 3 ) );
    bool waitForEventState( EventState state, const std::chrono::seconds duration = std::chrono::seconds( 3 ) );

    void resetSource();
    
    //
    // MediaPlayerEngineInterface executor methods
    //
    void executePlaybackStarted();
    void executePlaybackFinished();
    void executePlaybackPaused();
    void executePlaybackResumed();
    void executePlaybackStopped();
    void executePlaybackError( const MediaPlayerEngineInterface::ErrorType& type, const std::string& error );
    
    friend std::ostream& operator<<(std::ostream& stream, const EventState& state);

private:
    std::shared_ptr<aace::alexa::AudioChannel> m_audioChannelPlatformInterface;
    std::shared_ptr<aace::alexa::MediaPlayer> m_mediaPlayerPlatformInterface;
    std::shared_ptr<aace::alexa::Speaker> m_speakerPlatformInterface;

    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> m_observer;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> m_attachmentReader;
    
    alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus m_status;
    std::shared_ptr<std::istream> m_stream;
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId m_currentId;
    bool m_repeat;
    std::string m_url;
    std::chrono::milliseconds m_savedOffset;
    
    std::condition_variable m_eventState_cv;
    EventState m_expectedEventState;
    EventState m_eventState;

    // executor used to send asynchronous events back to observer
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    // global counter for media source id
    static SourceId s_nextId;

    // mutex for blocking setSource calls
    std::mutex m_mutex;

    // wait condition
    std::condition_variable m_trigger;    
};

inline std::ostream& operator<<(std::ostream& stream, const AudioChannelEngineImpl::EventState& state) {
    switch (state) {
        case AudioChannelEngineImpl::EventState::NONE:
            stream << "NONE";
            break;
        case AudioChannelEngineImpl::EventState::PLAYBACK_STARTED:
            stream << "PLAYBACK_STARTED";
            break;
        case AudioChannelEngineImpl::EventState::PLAYBACK_FINISHED:
            stream << "PLAYBACK_FINISHED";
            break;
        case AudioChannelEngineImpl::EventState::PLAYBACK_PAUSED:
            stream << "PLAYBACK_PAUSED";
            break;
        case AudioChannelEngineImpl::EventState::PLAYBACK_RESUMED:
            stream << "PLAYBACK_RESUMED";
            break;
        case AudioChannelEngineImpl::EventState::PLAYBACK_STOPPED:
            stream << "PLAYBACK_STOPPED";
            break;
        case AudioChannelEngineImpl::EventState::PLAYBACK_ERROR:
            stream << "PLAYBACK_ERROR";
            break;
    }
    return stream;
}

} // aace::engine::alexa
} // aace::engine
} // aace

#endif // AACE_ENGINE_ALEXA_AUDIO_CHANNEL_ENGINE_IMPL_H

