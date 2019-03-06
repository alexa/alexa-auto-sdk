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

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AudioChannelEngineImpl");

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::s_nextId =
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::ERROR;

AudioChannelEngineImpl::AudioChannelEngineImpl( std::shared_ptr<aace::alexa::AudioChannel> audioChannelPlatformInterface, const std::string& name ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(name),
    m_audioChannelPlatformInterface( audioChannelPlatformInterface ),
    m_status( alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus::OK ),
    m_stream( nullptr ),
    m_currentId( ERROR ),
    m_savedOffset( std::chrono::milliseconds( 0 ) ),
    m_name( name ),
    m_pendingEventState( PendingEventState::NONE ),
    m_currentMediaState( MediaState::STOPPED ),
    m_mediaStateChangeInitiator( MediaStateChangeInitiator::NONE ) {
}

bool AudioChannelEngineImpl::initializeAudioChannel( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager )
{
    try
    {
        ThrowIfNull(speakerManager, "invalidSpeakerManager");
        m_speakerManager = speakerManager;
    
        m_mediaPlayerPlatformInterface = m_audioChannelPlatformInterface->getMediaPlayer();
        ThrowIfNull( m_mediaPlayerPlatformInterface, "invalidMediaPlayerInterface" );

        m_speakerPlatformInterface = m_audioChannelPlatformInterface->getSpeaker();
        ThrowIfNull( m_speakerPlatformInterface, "invalidSpeakerInterface" );

        // add the speaker impl to the speaker manager
        m_speakerManager->addSpeaker( shared_from_this() );

        // set the media player engine interface
        m_mediaPlayerPlatformInterface->setEngineInterface( shared_from_this() );

        // set the speaker engine interface
        m_speakerPlatformInterface->setEngineInterface( shared_from_this() );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

void AudioChannelEngineImpl::doShutdown()
{
    m_executor.shutdown();

    // reset the media player engine interface
    if( m_mediaPlayerPlatformInterface != nullptr ) {
        m_mediaPlayerPlatformInterface->setEngineInterface(nullptr);
    }

    // reset the speaker engine interface
    if( m_speakerPlatformInterface != nullptr ) {
        m_speakerPlatformInterface->setEngineInterface(nullptr);
    }

    // reset the media observer reference
    m_observer.reset();
    
    // reset speaker manager reference
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
        AACE_WARN(LX(TAG,"sendEvent").d("reason","unhandledEventState").d("state",state));
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
        AACE_VERBOSE(LX(TAG,"executeMediaStateChanged").d("currentState",m_currentMediaState).d("newState",state).d("pendingEvent",m_pendingEventState).d("id",id));

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
        AACE_ERROR(LX(TAG,"executeMediaStateChanged").d("reason", ex.what()).d("currentState",m_currentMediaState).d("newState",state).d("pendingEvent",m_pendingEventState));
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
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackError( id, static_cast<alexaClientSDK::avsCommon::utils::mediaPlayer::ErrorType>( error ), description );
        }
        
        m_currentId = ERROR;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executeMediaError").d("reason", ex.what()));
    }
}

void AudioChannelEngineImpl::handlePrePlaybackStarted( SourceId id )
{
    AACE_DEBUG(LX(TAG,"handlePrePlaybackStarted").d("Event","Handling pre-playback started"));
}

void AudioChannelEngineImpl::handlePostPlaybackStarted( SourceId id )
{
    AACE_DEBUG(LX(TAG,"handlePostPlaybackStarted").d("Event","Handling post-playback started"));
}

void AudioChannelEngineImpl::handlePrePlaybackFinished( SourceId id )
{
    AACE_DEBUG(LX(TAG,"handlePrePlaybackFinished").d("Event","Handling pre-playback finished"));
}

void AudioChannelEngineImpl::handlePostPlaybackFinished( SourceId id ) 
{
    AACE_DEBUG(LX(TAG,"handlePostPlaybackFinished").d("Event","Handling post-playback finished"));
}

void AudioChannelEngineImpl::executePlaybackStarted( SourceId id )
{
    try
    {
        handlePrePlaybackStarted( id );
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackStarted( id );
        }
        handlePostPlaybackStarted( id );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackStarted").d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackFinished( SourceId id )
{
    try
    {
        handlePrePlaybackFinished( id );
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackFinished( id );
        }
        
        // save the player offset
        m_savedOffset = std::chrono::milliseconds( m_mediaPlayerPlatformInterface->getPosition() );

        m_currentId = ERROR;
        handlePostPlaybackFinished( id );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackFinished").d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackPaused( SourceId id )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        // save the player offset
        m_savedOffset = std::chrono::milliseconds( m_mediaPlayerPlatformInterface->getPosition() );

        if( m_observer != nullptr ) {
            m_observer->onPlaybackPaused( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackPaused").d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackResumed( SourceId id )
{
    try
    {
        ALEXA_METRIC(LX(TAG, "executePlaybackResumed").d("channelName", m_name), aace::engine::alexa::AlexaMetrics::Location::PLAYBACK_RESUMED);
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackResumed( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackResumed").d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackStopped( SourceId id )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );

        // save the player offset
        m_savedOffset = std::chrono::milliseconds( m_mediaPlayerPlatformInterface->getPosition() );

        if( m_observer != nullptr ) {
            m_observer->onPlaybackStopped( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackStopped").d("reason", ex.what()).d("expectedState",m_pendingEventState));
    }
}

void AudioChannelEngineImpl::executePlaybackError( SourceId id, MediaError error, const std::string& description )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackError( id, static_cast<alexaClientSDK::avsCommon::utils::mediaPlayer::ErrorType>( error ), description );
        }
        
        m_currentId = ERROR;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackError").d("reason", ex.what()));
    }
}

void AudioChannelEngineImpl::executeBufferUnderrun( SourceId id )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( m_observer != nullptr ) {
            m_observer->onBufferUnderrun( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executeBufferUnderrun").d("reason", ex.what()));
    }
}

void AudioChannelEngineImpl::executeBufferRefilled( SourceId id )
{
    try
    {
        ThrowIf( id == ERROR, "invalidSource" );
        
        if( m_observer != nullptr ) {
            m_observer->onBufferRefilled( id );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executeBufferRefilled").d("reason", ex.what()));
    }
}

ssize_t AudioChannelEngineImpl::read( char* data, const size_t size )
{
    try
    {
        if( m_attachmentReader != nullptr )
        {
            ssize_t count = m_attachmentReader->read( static_cast<void*>( data ), size, &m_status, std::chrono::milliseconds(100) );
            
            if( m_status >= alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus::CLOSED ) {
                m_closed = true;
            }
            
            return count;
        }
        else if( m_stream != nullptr )
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
        else {
            Throw( "invalidReadSource" );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"read").d("reason", ex.what()));
        m_closed = true;
        return 0;
    }
}

bool AudioChannelEngineImpl::isRepeating() {
    return m_repeat;
}

bool AudioChannelEngineImpl::isClosed() {
    return m_closed;
}

void AudioChannelEngineImpl::resetSource()
{
    m_attachmentReader.reset();
    m_stream.reset();
    m_status = alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus::OK;
    m_repeat = false;
    m_closed = false;
    m_currentId = ERROR;
    m_pendingEventState = PendingEventState::NONE;
    m_currentMediaState = MediaState::STOPPED;
    m_mediaStateChangeInitiator = MediaStateChangeInitiator::NONE;
    m_url.clear();
    m_savedOffset = std::chrono::milliseconds( 0 );
}

//
// aace::engine::SpeakerEngineInterface
//

void AudioChannelEngineImpl::onLocalVolumeSet( int8_t volume )
{
    if( m_speakerManager != nullptr ) {
        m_speakerManager->setVolume( getSpeakerType(), volume );
    }
}

void AudioChannelEngineImpl::onLocalMuteSet( bool mute )
{
    if( m_speakerManager != nullptr ) {
        m_speakerManager->setMute( getSpeakerType(), mute );
    }
}

//
// alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
//

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource( std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader, const alexaClientSDK::avsCommon::utils::AudioFormat* format )
{
    try
    {
        AACE_DEBUG(LX(TAG,"setSource").d("type","attachment"));

        resetSource();
    
        m_attachmentReader = std::move( attachmentReader );
        m_currentId = nextId();

        ThrowIfNot( m_mediaPlayerPlatformInterface->prepare(), "platformMediaPlayerPrepareFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setSource").d("reason", ex.what()));
        resetSource();
    }
    
    return m_currentId;
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource( std::shared_ptr<std::istream> stream, bool repeat )
{
    try
    {
        AACE_DEBUG(LX(TAG,"setSource").d("type","stream"));

        resetSource();

        ThrowIfNot( stream->good(), "invalidStream" );
        
        m_stream = std::move( stream );
        m_repeat = repeat;
        m_currentId = nextId();

        ThrowIfNot( m_mediaPlayerPlatformInterface->prepare(), "platformMediaPlayerPrepareFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setSource").d("reason", ex.what()).d("expectedState",m_pendingEventState));
        resetSource();
    }

    return m_currentId;
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource( const std::string& url, std::chrono::milliseconds offset )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_DEBUG(LX(TAG,"setSource").d("type","url").sensitive("url", url));

        resetSource();
        
        m_url = url;
        m_currentId = nextId();

        ThrowIfNot( m_mediaPlayerPlatformInterface->prepare( m_url ), "platformMediaPlayerPrepareFailed" );
        ThrowIfNot( m_mediaPlayerPlatformInterface->setPosition( offset.count() ), "platformMediaPlayerSetPositionFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setSource").d("reason", ex.what()));
        resetSource();
    }

    return m_currentId;
}

bool AudioChannelEngineImpl::play( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_VERBOSE(LX(TAG,"play").d("id",id));

        ThrowIfNot( validateSource( id ), "invalidSource" );

        // return false if audio is already playing
        ReturnIf( m_currentMediaState == MediaState::PLAYING || m_currentMediaState == MediaState::BUFFERING, false );

        // return false if play() was already called but no callback has been made yet
        ReturnIf( m_pendingEventState == PendingEventState::PLAYBACK_STARTED, false );

        // send the pending event
        sendPendingEvent();

        //invoke the platform interface play method
        ALEXA_METRIC(LX(TAG, "play").d("channelName", m_name), aace::engine::alexa::AlexaMetrics::Location::PLAYBACK_REQUEST_START);
        ThrowIfNot( m_mediaPlayerPlatformInterface->play(), "platformMediaPlayerPlayFailed" );

        // set the expected pending event state
        m_pendingEventState = PendingEventState::PLAYBACK_STARTED;
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"play").d("reason", ex.what()).d("expectedState",m_pendingEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::stop( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_VERBOSE(LX(TAG,"stop").d("id",id));

        ThrowIfNot( validateSource( id ), "invalidSource" );

        // return false if audio is already stopped
        ReturnIf( m_mediaStateChangeInitiator == MediaStateChangeInitiator::STOP, false );

        // send the pending event
        sendPendingEvent();

        // invoke the platform interface stop method
        ThrowIfNot( m_mediaPlayerPlatformInterface->stop(), "platformMediaPlayerStopFailed" );
        
        // set the expected pending event state and media offset
        m_pendingEventState = PendingEventState::PLAYBACK_STOPPED;

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"stop").d("reason", ex.what()).d("expectedState",m_pendingEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::pause( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_VERBOSE(LX(TAG,"pause").d("id",id));

        ThrowIfNot( validateSource( id ), "invalidSource" );
        ReturnIf( id == ERROR, true );

        // return false if audio is not playing/starting/resuming
        ReturnIf( m_currentMediaState == MediaState::STOPPED 
                && m_pendingEventState != PendingEventState::PLAYBACK_STARTED 
                && m_pendingEventState != PendingEventState::PLAYBACK_RESUMED, false );

        // send the pending event
        sendPendingEvent();
        
        // invoke the platform interface pause method
        ThrowIfNot( m_mediaPlayerPlatformInterface->pause(), "platformMediaPlayerPauseFailed" );
        
        // set the expected pending event state and media offset
        m_pendingEventState = PendingEventState::PLAYBACK_PAUSED;

        // if the current media state is already stopped then send up the pending event now
        if( m_currentMediaState == MediaState::STOPPED ) {
            sendPendingEvent();
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"pause").d("reason", ex.what()).d("expectedState",m_pendingEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::resume( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    std::unique_lock<std::mutex> lock( m_mutex );

    try
    {
        AACE_VERBOSE(LX(TAG,"resume").d("id",id));

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
        ALEXA_METRIC(LX(TAG, "resume").d("channelName", m_name), aace::engine::alexa::AlexaMetrics::Location::PLAYBACK_REQUEST_RESUME);
        ThrowIfNot( m_mediaPlayerPlatformInterface->resume(), "platformMediaPlayerResumeFailed" );
        
        // set the expected pending event state
        m_pendingEventState = PendingEventState::PLAYBACK_RESUMED;

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"resume").d("reason", ex.what()).d("expectedState",m_pendingEventState));
        return false;
    }
}

std::chrono::milliseconds AudioChannelEngineImpl::getOffset( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    try
    {
        ReturnIf( m_currentId == ERROR || m_currentId != id, m_savedOffset );
        
        std::chrono::milliseconds offset = std::chrono::milliseconds( m_mediaPlayerPlatformInterface->getPosition() );
        ThrowIf( offset.count() < 0, "invalidMediaTime" );

        return offset;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"getOffset").d("reason", ex.what()));
        return std::chrono::milliseconds( 0 );
    }
}

uint64_t AudioChannelEngineImpl::getNumBytesBuffered() {
    return 0;
}

void AudioChannelEngineImpl::setObserver(std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> observer) {
    m_observer = observer;
}

//
// alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface
//
bool AudioChannelEngineImpl::setVolume( int8_t volume ) {
    return m_speakerPlatformInterface->setVolume( volume );
}

bool AudioChannelEngineImpl::adjustVolume( int8_t delta ) {
    return m_speakerPlatformInterface->adjustVolume( delta );
}

bool AudioChannelEngineImpl::setMute( bool mute ) {
    return m_speakerPlatformInterface->setMute( mute );
}

bool AudioChannelEngineImpl::getSpeakerSettings( alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings ) {
    settings->volume = m_speakerPlatformInterface->getVolume();
    settings->mute = m_speakerPlatformInterface->isMuted();
    return true;
}

alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type AudioChannelEngineImpl::getSpeakerType() {
    return static_cast<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type>( m_audioChannelPlatformInterface->getSpeakerType() );
}

bool AudioChannelEngineImpl::validateSource( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    try
    {
        ThrowIf( m_currentId != id, "invalidSource" )
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"validateSource").d("reason", ex.what()).d("id",id).d("currentId",m_currentId));
        return false;
    }
}

} // aace::engine::alexa
} // aace::engine
} // aace

