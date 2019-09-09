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

#include "AACE/Engine/Alexa/AudioChannelEngineImpl.h"
#include "AACE/Engine/Alexa/AlexaMetrics.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

static const uint8_t MAX_SPEAKER_VOLUME = 100;
static const uint8_t MIN_SPEAKER_VOLUME = 0;
static const uint8_t DEFAULT_SPEAKER_VOLUME = 50;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AudioChannelEngineImpl");

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::s_nextId =
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::ERROR;

AudioChannelEngineImpl::AudioChannelEngineImpl( alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type speakerType ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
    m_speakerType(speakerType),
    m_currentId( ERROR ),
    m_savedOffset( std::chrono::milliseconds( 0 ) ),
    m_muted( false ),
    m_volume( DEFAULT_SPEAKER_VOLUME ),
    m_pendingEventState( PendingEventState::NONE ),
    m_currentMediaState( MediaState::STOPPED ),
    m_mediaStateChangeInitiator( MediaStateChangeInitiator::NONE ) {
}

bool AudioChannelEngineImpl::initializeAudioChannel(
    std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> audioOutputChannel,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager ) {
    
    try
    {
        ThrowIfNull( audioOutputChannel, "invalidAudioOutputChannel" );
        ThrowIfNull( speakerManager, "invalidSpeakerManager");

        // save the audio channel reference
        m_audioOutputChannel = audioOutputChannel;
    
        // set the audio output channel engine interface
        audioOutputChannel->setEngineInterface( shared_from_this() );
    
        // add the speaker impl to the speaker manager
        speakerManager->addSpeaker( shared_from_this() );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AudioChannelEngineImpl::doShutdown()
{
    m_executor.shutdown();

    // reset the audio output channel engine interface
    if( m_audioOutputChannel != nullptr ) {
        m_audioOutputChannel->setEngineInterface( nullptr );
        m_audioOutputChannel.reset();
    }

    // reset the media observer reference
    m_mediaPlayerObserver.reset();
    // reset the speaker manager
    m_speakerManager.reset();
}

void AudioChannelEngineImpl::sendPendingEvent()
{
    if( m_pendingEventState != PendingEventState::NONE ) {
        sendEvent( m_pendingEventState );
        m_pendingEventState = PendingEventState::NONE;
    }
}

void AudioChannelEngineImpl::sendEvent( PendingEventState state )
{
    SourceId id = m_currentId;

    if( state == PendingEventState::PLAYBACK_STARTED ) {
        m_executor.submit([this,id] {
            executePlaybackStarted( id );
        });
    }
    else if( state == PendingEventState::PLAYBACK_RESUMED ) {
        m_executor.submit([this,id] {
            executePlaybackResumed( id );
        });
    }
    else if( state == PendingEventState::PLAYBACK_STOPPED ) {
        m_executor.submit([this,id] {
            executePlaybackStopped( id );
        });
    }
    else if( state == PendingEventState::PLAYBACK_PAUSED ) {
        m_executor.submit([this,id] {
            executePlaybackPaused( id );
        });
    }
    else {
        AACE_WARN(LX(TAG).d("reason","unhandledEventState").d("state",state));
    }
}

//
// aace::engine::MediaPlayerEngineInterface
//

void AudioChannelEngineImpl::onMediaStateChanged( MediaState state )
{
    auto id = m_currentId;
    m_executor.submit([this,id,state] {
        executeMediaStateChanged( id, state );
    });
}

void AudioChannelEngineImpl::executeMediaStateChanged( SourceId id, MediaState state )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_VERBOSE(LX(TAG).d("currentState",m_currentMediaState).d("newState",state).d("pendingEvent",m_pendingEventState).d("id",id));

        // return if the current media state is the same as the new state and no pending event
        if( m_currentMediaState == state && m_pendingEventState == PendingEventState::NONE ) {
            return;
        }
    
        // handle media state switch to PLAYING
        if( state == MediaState::PLAYING )
        {
            // if the current state is STOPPED then pending event should be set to either
            // PLAYBACK_STARTED or PLAYBACK_RESUMED... otherwise the platform is attempting
            // to change the media state at an unexpected time!
            if( m_currentMediaState == MediaState::STOPPED )
            {
                if( m_pendingEventState == PendingEventState::PLAYBACK_STARTED ) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::PLAY;
                    executePlaybackStarted( id );
                    m_pendingEventState = PendingEventState::NONE;
                }
                else if( m_pendingEventState == PendingEventState::PLAYBACK_RESUMED ) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::RESUME;
                    executePlaybackResumed( id );
                    m_pendingEventState = PendingEventState::NONE;
                }
                else {
                    Throw( "unexpectedPendingEventState" );
                }
            }
            
            // if the current state is buffering then the platform is notifying us that
            // playback has resumed after filling its media buffer.
            else if( m_currentMediaState == MediaState::BUFFERING ) {
                executeBufferRefilled( id );
            }
            
            else {
                Throw( "unexpectedMediaState" );
            }
        }

        // handle media state switch to STOPPED
        else if( state == MediaState::STOPPED )
        {
            // if the current state is PLAYING the pending event should be set to either
            // PLAYBACK_STOPPED or PLAYBACK_PAUSED. If the pending state is NONE, then
            // we assume that media state is indicating playback has finished.
            if( m_currentMediaState == MediaState::PLAYING )
            {
                if( m_pendingEventState == PendingEventState::PLAYBACK_STOPPED ) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::STOP;
                    executePlaybackStopped( id );
                    m_pendingEventState = PendingEventState::NONE;
                }
                else if( m_pendingEventState == PendingEventState::PLAYBACK_PAUSED ) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::PAUSE;
                    executePlaybackPaused( id );
                    m_pendingEventState = PendingEventState::NONE;
                }
                else if( m_pendingEventState == PendingEventState::NONE ) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::NONE;
                    executePlaybackFinished( id );
                    m_pendingEventState = PendingEventState::NONE;
                }
                else {
                    Throw( "unexpectedPendingEventState" );
                }
            }
            
            // if the current media state is BUFFERING the pending event should be set to either
            // PLAYBACK_STOPPED or PLAYBACK_PAUSED.
            else if( m_currentMediaState == MediaState::BUFFERING )
            {
                if( m_pendingEventState == PendingEventState::PLAYBACK_STOPPED ) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::STOP;
                    executePlaybackStopped( id );
                    m_pendingEventState = PendingEventState::NONE;
                }
                else if( m_pendingEventState == PendingEventState::PLAYBACK_PAUSED ) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::PAUSE;
                    executePlaybackPaused( id );
                    m_pendingEventState = PendingEventState::NONE;
                }
                else {
                    Throw( "unexpectedPendingEventState" );
                }
            }

            // if the current media state is STOPPED the pending event should be set to PLAYBACK_STOPPED
            // if we are transitioning from paused to stopped
            else if( m_currentMediaState == MediaState::STOPPED )
            {
                if( m_pendingEventState == PendingEventState::PLAYBACK_STOPPED ) {
                    m_mediaStateChangeInitiator = MediaStateChangeInitiator::STOP;
                    executePlaybackStopped( id );
                    m_pendingEventState = PendingEventState::NONE;
                }
                else {
                    Throw( "unexpectedPendingEventState" );
                }
            }

            // if current state is anything else it is considered an error, since the platform
            // is only allowed to transition to STOPPED if it currently PLAYING or STOPPED from pause().
            else {
                Throw( "unexpectedMediaState" );
            }
        }
        
        // handle media state switch to BUFFERING
        else if( state == MediaState::BUFFERING )
        {
            // if the pending event is PLAYBACK_STARTED then we ignore the media state change to BUFFERING
            // since media is considering to be in a loading state until set to PLAYING
            if( m_pendingEventState == PendingEventState::PLAYBACK_STARTED ) {
                return;
            }
            
            // if the pending event is is PLAYBACK_RESUMED then send the resumed event to AVS before sending
            // the buffer underrun event
            else if( m_pendingEventState == PendingEventState::PLAYBACK_RESUMED ) {
                executePlaybackResumed( id );
                executeBufferUnderrun( id );
                m_pendingEventState = PendingEventState::NONE;
            }
            
            // handle condition when there is no pending event
            else if( m_pendingEventState == PendingEventState::NONE  )
            {
                // if the current state is PLAYING then send the buffer underrun event, otherwise
                // we choose to ignore the BUFFERING state...
                if( m_currentMediaState == MediaState::PLAYING ) {
                    executeBufferUnderrun( id );
                }
                else {
                    return;
                }
            }
            else {
                Throw( "unexpectedMediaStateForBuffering" );
            }
        }
        
        m_currentMediaState = state;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("currentState",m_currentMediaState).d("newState",state).d("pendingEvent",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::onMediaError( MediaError error, const std::string& description )
{
    auto id = m_currentId;
    m_executor.submit([this,id,error,description] {
        executeMediaError( id, error, description );
    });
    m_pendingEventState = PendingEventState::NONE;
}

void AudioChannelEngineImpl::executeMediaError( SourceId id, MediaError error, const std::string& description )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( auto m_mediaPlayerObserver_lock = m_mediaPlayerObserver.lock() ) {
            m_mediaPlayerObserver_lock->onPlaybackError( id, static_cast<alexaClientSDK::avsCommon::utils::mediaPlayer::ErrorType>( error ), description );
        }
        
        m_currentId = ERROR;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AudioChannelEngineImpl::handlePrePlaybackStarted( SourceId id ){
    AACE_DEBUG(LX(TAG).d("Event","Handling pre-playback started"));
}

void AudioChannelEngineImpl::handlePostPlaybackStarted( SourceId id ){
    AACE_DEBUG(LX(TAG).d("Event","Handling post-playback started"));
}

void AudioChannelEngineImpl::handlePrePlaybackFinished( SourceId id ){
    AACE_DEBUG(LX(TAG).d("Event","Handling pre-playback finished"));
}

void AudioChannelEngineImpl::handlePostPlaybackFinished( SourceId id ) {
    AACE_DEBUG(LX(TAG).d("Event","Handling post-playback finished"));
}

void AudioChannelEngineImpl::executePlaybackStarted( SourceId id )
{
    try
    {
        handlePrePlaybackStarted( id );
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( auto m_mediaPlayerObserver_lock = m_mediaPlayerObserver.lock() ) {
            m_mediaPlayerObserver_lock->onPlaybackStarted( id );
        }
        
        handlePostPlaybackStarted( id );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackFinished( SourceId id )
{
    try
    {
        handlePrePlaybackFinished( id );
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( auto m_mediaPlayerObserver_lock = m_mediaPlayerObserver.lock() ) {
            m_mediaPlayerObserver_lock->onPlaybackFinished( id );
        }
        
        // save the player offset
        m_savedOffset = std::chrono::milliseconds( m_audioOutputChannel->getPosition() );

        m_currentId = ERROR;
        
        handlePostPlaybackFinished( id );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackPaused( SourceId id )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        // save the player offset
        m_savedOffset = std::chrono::milliseconds( m_audioOutputChannel->getPosition() );

        if( auto m_mediaPlayerObserver_lock = m_mediaPlayerObserver.lock() ) {
            m_mediaPlayerObserver_lock->onPlaybackPaused( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackResumed( SourceId id )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( auto m_mediaPlayerObserver_lock = m_mediaPlayerObserver.lock() ) {
            m_mediaPlayerObserver_lock->onPlaybackResumed( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackStopped( SourceId id )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );

        // save the player offset
        m_savedOffset = std::chrono::milliseconds( m_audioOutputChannel->getPosition() );

        if( auto m_mediaPlayerObserver_lock = m_mediaPlayerObserver.lock() ) {
            m_mediaPlayerObserver_lock->onPlaybackStopped( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackError( SourceId id, MediaError error, const std::string& description )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( auto m_mediaPlayerObserver_lock = m_mediaPlayerObserver.lock() ) {
            m_mediaPlayerObserver_lock->onPlaybackError( id, static_cast<alexaClientSDK::avsCommon::utils::mediaPlayer::ErrorType>( error ), description );
        }
        
        m_currentId = ERROR;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AudioChannelEngineImpl::executeBufferUnderrun( SourceId id )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( auto m_mediaPlayerObserver_lock = m_mediaPlayerObserver.lock() ) {
            m_mediaPlayerObserver_lock->onBufferUnderrun( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AudioChannelEngineImpl::executeBufferRefilled( SourceId id )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( auto m_mediaPlayerObserver_lock = m_mediaPlayerObserver.lock() ) {
            m_mediaPlayerObserver_lock->onBufferRefilled( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AudioChannelEngineImpl::resetSource()
{
    m_currentId = ERROR;
    m_pendingEventState = PendingEventState::NONE;
    m_currentMediaState = MediaState::STOPPED;
    m_mediaStateChangeInitiator = MediaStateChangeInitiator::NONE;
    m_url.clear();
    m_savedOffset = std::chrono::milliseconds( 0 );
}

//
// alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
//

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource( std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader, const alexaClientSDK::avsCommon::utils::AudioFormat* format )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_DEBUG(LX(TAG).d("type","attachment"));

        resetSource();
    
        m_currentId = nextId();

        ThrowIfNot( m_audioOutputChannel->prepare( AttachmentReaderAudioStream::create( attachmentReader, format ), false ), "audioOutputChannelSetStreamFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        resetSource();
    }
    
    return m_currentId;
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource( std::shared_ptr<std::istream> stream, bool repeat )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_DEBUG(LX(TAG).d("type","stream"));

        resetSource();

        ThrowIfNot( stream->good(), "invalidStream" );
        
        m_currentId = nextId();

        ThrowIfNot( m_audioOutputChannel->prepare( IStreamAudioStream::create( stream ), repeat ), "audioOutputChannelSetStreamFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
        resetSource();
    }

    return m_currentId;
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource( const std::string& url, std::chrono::milliseconds offset, bool repeat )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_DEBUG(LX(TAG).d("type","url").sensitive("url", url));

        resetSource();
        
        m_url = url;
        m_currentId = nextId();

        ThrowIfNot( m_audioOutputChannel->prepare( m_url, repeat ), "platformMediaPlayerPrepareFailed" );
        ThrowIfNot( m_audioOutputChannel->setPosition( offset.count() ), "platformMediaPlayerSetPositionFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        resetSource();
    }

    return m_currentId;
}

bool AudioChannelEngineImpl::play( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_VERBOSE(LX(TAG).d("id",id));

        ThrowIfNot( validateSource( id ), "invalidSource" );

        // return false if audio is already playing
        ReturnIf( m_currentMediaState == MediaState::PLAYING || m_currentMediaState == MediaState::BUFFERING, false );

        // return false if play() was already called but no callback has been made yet
        ReturnIf( m_pendingEventState == PendingEventState::PLAYBACK_STARTED, false );

        // send the pending event
        sendPendingEvent();

        //invoke the platform interface play method
        ThrowIfNot( m_audioOutputChannel->play(), "platformMediaPlayerPlayFailed" );

        // set the expected pending event state
        m_pendingEventState = PendingEventState::PLAYBACK_STARTED;
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::stop( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_VERBOSE(LX(TAG).d("id",id));

        ThrowIfNot( validateSource( id ), "invalidSource" );

        // return false if audio is already stopped
        ReturnIf( m_mediaStateChangeInitiator == MediaStateChangeInitiator::STOP, false );

        // send the pending event
        sendPendingEvent();

        // invoke the platform interface stop method
        if ( m_audioOutputChannel != nullptr ) {
            ThrowIfNot( m_audioOutputChannel->stop(), "platformMediaPlayerStopFailed" );
        }

        // set the expected pending event state and media offset
        m_pendingEventState = PendingEventState::PLAYBACK_STOPPED;

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::pause( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_VERBOSE(LX(TAG).d("id",id));

        ThrowIfNot( validateSource( id ), "invalidSource" );
        ReturnIf( id == ERROR, true );

        // return false if audio is not playing/starting/resuming
        ReturnIf( m_currentMediaState == MediaState::STOPPED 
                && m_pendingEventState != PendingEventState::PLAYBACK_STARTED 
                && m_pendingEventState != PendingEventState::PLAYBACK_RESUMED, false );

        // send the pending event
        sendPendingEvent();
        
        // invoke the platform interface pause method
        ThrowIfNot( m_audioOutputChannel->pause(), "platformMediaPlayerPauseFailed" );
        
        // set the expected pending event state and media offset
        m_pendingEventState = PendingEventState::PLAYBACK_PAUSED;

        // if the current media state is already stopped then send up the pending event now
        if( m_currentMediaState == MediaState::STOPPED ) {
            sendPendingEvent();
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::resume( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_VERBOSE(LX(TAG).d("id",id));

        ThrowIfNot( validateSource( id ), "invalidSource" );

        // return false if audio is not paused
        ReturnIf( m_mediaStateChangeInitiator != MediaStateChangeInitiator::PAUSE, false );

        // return false if audio is already playing
        ReturnIf( m_currentMediaState == MediaState::PLAYING || m_currentMediaState == MediaState::BUFFERING, false );

        // return false if resume() was already called but no callback has been made yet
        ReturnIf( m_pendingEventState == PendingEventState::PLAYBACK_RESUMED, false );
        
        // send the pending event
        sendPendingEvent();

        // invoke the platform interface resume method
        ThrowIfNot( m_audioOutputChannel->resume(), "platformMediaPlayerResumeFailed" );
        
        // set the expected pending event state
        m_pendingEventState = PendingEventState::PLAYBACK_RESUMED;

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("expectedState",m_pendingEventState));
        return false;
    }
}

std::chrono::milliseconds AudioChannelEngineImpl::getOffset( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    try
    {
        ReturnIf( m_currentId == ERROR || m_currentId != id, m_savedOffset );
        
        std::chrono::milliseconds offset = std::chrono::milliseconds( m_audioOutputChannel->getPosition() );
        ThrowIf( offset.count() < 0, "invalidMediaTime" );

        return offset;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::chrono::milliseconds( 0 );
    }
}

uint64_t AudioChannelEngineImpl::getNumBytesBuffered() {
    return 0;
}

void AudioChannelEngineImpl::setObserver( std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> observer )
{
    try
    {
        if( observer != nullptr )
        {
            ThrowIfNotNull( m_mediaPlayerObserver.lock(), "mediaPlayerObserverAlreadySet" );
            m_mediaPlayerObserver = observer;
        }
        else {
            m_mediaPlayerObserver.reset();
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

//
// alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface
//

bool AudioChannelEngineImpl::setVolume( int8_t volume )
{
    try
    {
        if (volume < MIN_SPEAKER_VOLUME || MAX_SPEAKER_VOLUME < volume) {
            Throw("Volume is out of range");
        }
        ThrowIfNot( m_audioOutputChannel->volumeChanged( (float) volume / MAX_SPEAKER_VOLUME ), "audioOutputChannelSetVolumeFailed" );
        
        m_volume = volume;
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioChannelEngineImpl::adjustVolume( int8_t delta )
{
    try
    {
        ThrowIfNot( setVolume( m_volume + delta ), "setVolumeFailed" );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioChannelEngineImpl::setMute( bool mute )
{
    try
    {
        m_muted = mute;

        ThrowIfNot( m_audioOutputChannel->mutedStateChanged( mute ?
            aace::engine::audio::AudioOutputChannelInterface::MutedState::MUTED :
            aace::engine::audio::AudioOutputChannelInterface::MutedState::UNMUTED ), "audioOutputChannelSetMuteFailed" );
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AudioChannelEngineImpl::getSpeakerSettings( alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings )
{
    try
    {
        settings->volume = m_volume;
        settings->mute = m_muted;
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type AudioChannelEngineImpl::getSpeakerType() {
    return m_speakerType;
}

bool AudioChannelEngineImpl::validateSource( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    try
    {
        ThrowIf( m_currentId != id, "invalidSource" )
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("id",id).d("currentId",m_currentId));
        return false;
    }
}

//
// AttachmentReaderStream
//

AttachmentReaderAudioStream::AttachmentReaderAudioStream(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    Encoding encoding ) :
        m_attachmentReader( attachmentReader ),
        m_status( alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus::OK ),
        m_closed( false ),
        m_encoding( encoding ) {
}

std::shared_ptr<AttachmentReaderAudioStream> AttachmentReaderAudioStream::create(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    const alexaClientSDK::avsCommon::utils::AudioFormat* format ) {

    Encoding encoding = Encoding::MP3;
    
    if( format != nullptr )
    {
        switch( format->encoding )
        {
            case alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::LPCM:
                encoding = Encoding::LPCM;
                break;
            case alexaClientSDK::avsCommon::utils::AudioFormat::Encoding::OPUS:
                encoding = Encoding::OPUS;
                break;
        }
    }

    return std::shared_ptr<AttachmentReaderAudioStream>( new AttachmentReaderAudioStream( attachmentReader, encoding ) );
}

ssize_t AttachmentReaderAudioStream::read( char* data, const size_t size )
{
    try
    {
        ssize_t count = m_attachmentReader->read( static_cast<void*>( data ), size, &m_status, std::chrono::milliseconds(100) );
        
        if( m_status >= alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus::CLOSED ) {
            m_closed = true;
        }
        
        return count;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG+".AttachmentReaderAudioStream").d("reason", ex.what()));
        m_closed = true;
        return 0;
    }
}

bool AttachmentReaderAudioStream::isClosed() {
    return m_closed;
}

AttachmentReaderAudioStream::Encoding AttachmentReaderAudioStream::getEncoding() {
    return m_encoding;
}

//
// IStreamAudioStream
//

IStreamAudioStream::IStreamAudioStream( std::shared_ptr<std::istream> stream ) : m_stream( stream ), m_closed( false ) {
}

std::shared_ptr<IStreamAudioStream> IStreamAudioStream::create( std::shared_ptr<std::istream> stream ) {
    return std::shared_ptr<IStreamAudioStream>( new IStreamAudioStream( stream ) );
}

ssize_t IStreamAudioStream::read( char* data, const size_t size )
{
    try
    {
        if( m_stream->eof() ) {
            m_closed = true;
            return 0;
        }
    
        // read the data from the stream
        m_stream->read( data, size );
        ThrowIf( m_stream->bad(), "readFailed" );
        
        // get the number of bytes read
        ssize_t count = m_stream->gcount();
        
        m_stream->tellg(); // Don't remove otherwise the ReseourceStream used for Alerts/Timers won't work as expected.

        return count;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG+".IStreamAudioStream").d("reason", ex.what()));
        m_closed = true;
        return 0;
    }
}

bool IStreamAudioStream::isClosed() {
    return m_closed;
}

} // aace::engine::alexa
} // aace::engine
} // aace

