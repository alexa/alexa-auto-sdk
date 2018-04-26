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

#include <AVSCommon/Utils/LibcurlUtils/HTTPContentFetcherFactory.h>

#include "AACE/Engine/Alexa/AudioChannelEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AudioChannelEngineImpl");

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::s_nextId =
    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::ERROR;

AudioChannelEngineImpl::AudioChannelEngineImpl( std::shared_ptr<aace::alexa::AudioChannel> audioChannelPlatformInterface ) :
    m_audioChannelPlatformInterface( audioChannelPlatformInterface ),
    m_status( alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus::OK ),
    m_stream( nullptr ),
    m_currentId( ERROR ),
    m_savedOffset( std::chrono::milliseconds( 0 ) ),
    m_expectedEventState( EventState::NONE ),
    m_eventState( EventState::NONE ) {
    
    m_mediaPlayerPlatformInterface = audioChannelPlatformInterface->getMediaPlayer();
    m_speakerPlatformInterface = audioChannelPlatformInterface->getSpeaker();
}

AudioChannelEngineImpl::~AudioChannelEngineImpl() {
    m_executor.shutdown();
}

void AudioChannelEngineImpl::setEventState( EventState state ) {
    m_eventState = state;
    m_eventState_cv.notify_all();
    m_expectedEventState = EventState::NONE;
}

bool AudioChannelEngineImpl::waitForEventState( const std::chrono::seconds duration ) {
    return waitForEventState( m_expectedEventState, duration );
}

bool AudioChannelEngineImpl::waitForEventState( EventState state, const std::chrono::seconds duration )
{
    ReturnIf( state == EventState::NONE || state == m_eventState, true );
    
    std::unique_lock<std::mutex> lock( m_mutex );
    
    AACE_DEBUG(LX(TAG,"waitForEventState").d("state",state).d("expecting",m_expectedEventState));
    
    // reset the event state
    m_eventState = EventState::NONE;
    
    return m_eventState_cv.wait_for( lock, duration, [this, state]()
    {
        if( state != m_eventState ) {
            AACE_ERROR(LX(TAG,"waitForEventState").d("reason","timeout").d("state",state).d("expecting",m_expectedEventState));
            return false;
        }
        else {
            return true;
        }
    });
}

//
// aace::engine::MediaPlayerEngineInterface
//

void AudioChannelEngineImpl::onPlaybackStarted()
{
    m_executor.submit([this] {
        executePlaybackStarted();
    });
}

void AudioChannelEngineImpl::executePlaybackStarted()
{
    try
    {
        ThrowIf( m_expectedEventState != EventState::PLAYBACK_STARTED, "invalidExpectedState" );
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackStarted( m_currentId );
        }

        setEventState( EventState::PLAYBACK_STARTED );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackStarted").d("reason", ex.what()).d("expectedState",m_expectedEventState));
    }
}

void AudioChannelEngineImpl::onPlaybackFinished()
{
    m_executor.submit([this] {
        executePlaybackFinished();
    });
}

void AudioChannelEngineImpl::executePlaybackFinished()
{
    try
    {
        ThrowIf( m_expectedEventState != EventState::NONE, "invalidExpectedState" );
        ThrowIf( m_currentId == ERROR, "invalidSource" );
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackFinished( m_currentId );
        }
        
        m_currentId = ERROR;
        
        setEventState( EventState::PLAYBACK_FINISHED );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackFinished").d("reason", ex.what()).d("expectedState",m_expectedEventState));
    }
}

void AudioChannelEngineImpl::onPlaybackPaused()
{
    m_executor.submit([this] {
        executePlaybackPaused();
    });
}

void AudioChannelEngineImpl::executePlaybackPaused()
{
    try
    {
        ThrowIf( m_expectedEventState != EventState::PLAYBACK_PAUSED, "invalidExpectedState" );
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackPaused( m_currentId );
        }

        setEventState( EventState::PLAYBACK_PAUSED );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackPaused").d("reason", ex.what()).d("expectedState",m_expectedEventState));
    }
}

void AudioChannelEngineImpl::onPlaybackResumed()
{
    m_executor.submit([this] {
        executePlaybackResumed();
    });
}

void AudioChannelEngineImpl::executePlaybackResumed()
{
    try
    {
        ThrowIf( m_expectedEventState != EventState::PLAYBACK_RESUMED, "invalidExpectedState" );
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackResumed( m_currentId );
        }

        setEventState( EventState::PLAYBACK_RESUMED );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackResumed").d("reason", ex.what()).d("expectedState",m_expectedEventState));
    }
}

void AudioChannelEngineImpl::onPlaybackStopped()
{
    m_executor.submit([this] {
        executePlaybackStopped();
    });
}

void AudioChannelEngineImpl::executePlaybackStopped()
{
    try
    {
        ThrowIf( m_expectedEventState != EventState::PLAYBACK_STOPPED, "invalidExpectedState" );

        if( m_observer != nullptr ) {
            m_observer->onPlaybackStopped( m_currentId );
        }
    
        m_currentId = ERROR;

        setEventState( EventState::PLAYBACK_STOPPED );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackStopped").d("reason", ex.what()).d("expectedState",m_expectedEventState));
    }

}

void AudioChannelEngineImpl::onPlaybackError( const MediaPlayerEngineInterface::ErrorType& type, const std::string& error )
{
    m_executor.submit([this,type,error] {
        executePlaybackError( type, error );
    });
}

void AudioChannelEngineImpl::executePlaybackError( const MediaPlayerEngineInterface::ErrorType& type, const std::string& error )
{
    try
    {
        ThrowIf( m_currentId == ERROR, "invalidSource" );
        
        if( m_observer != nullptr ) {
            m_observer->onPlaybackError( m_currentId, static_cast<alexaClientSDK::avsCommon::utils::mediaPlayer::ErrorType>( type ), error );
        }
        
        m_currentId = ERROR;
        
        setEventState( EventState::PLAYBACK_ERROR );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"executePlaybackError").d("reason", ex.what()));
    }
}

ssize_t AudioChannelEngineImpl::read( char* data, const size_t size )
{
    try
    {
        if( m_attachmentReader != nullptr ) {
            ssize_t count = m_attachmentReader->read( static_cast<void*>( data ), size, &m_status );
            ThrowIf( count < 0, "readFailed" );
            return count;
        }
        else if( m_stream != nullptr )
        {
            ReturnIf( m_stream->eof(), 0 );
        
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
        return 0;
    }
}

bool AudioChannelEngineImpl::isRepeating() {
    return m_repeat;
}

void AudioChannelEngineImpl::resetSource()
{
    m_attachmentReader.reset();
    m_stream.reset();
    m_status = alexaClientSDK::avsCommon::avs::attachment::AttachmentReader::ReadStatus::OK;
    m_repeat = false;
    m_currentId = ERROR;
    m_eventState = EventState::NONE;
    m_expectedEventState = EventState::NONE;
    m_url.clear();
    m_savedOffset = std::chrono::milliseconds( 0 );
}

//
// alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
//

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource( std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader )
{
    try
    {
        AACE_DEBUG(LX(TAG,"setSource").d("type","attachment"));

        ThrowIfNot( waitForEventState(), "waitForEventStateTimeout" );

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

        ThrowIfNot( waitForEventState(), "waitForEventStateTimeout" );

        resetSource();

        ThrowIfNot( stream->good(), "invalidStream" );
        
        m_stream = std::move( stream );
        m_repeat = repeat;
        m_currentId = nextId();

        ThrowIfNot( m_mediaPlayerPlatformInterface->prepare(), "platformMediaPlayerPrepareFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setSource").d("reason", ex.what()).d("expectedState",m_expectedEventState));
        resetSource();
    }

    return m_currentId;
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId AudioChannelEngineImpl::setSource( const std::string& url, std::chrono::milliseconds offset )
{
    try
    {
        AACE_DEBUG(LX(TAG,"setSource").d("type","url").sensitive("url", url));

        ThrowIfNot( waitForEventState(), "waitForEventStateTimeout" );

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
    try
    {
        ThrowIf( m_expectedEventState != EventState::NONE, "invalidEventState" );
        ThrowIfNot( validateSource( id ), "invalidSource" );

        ThrowIfNot( m_mediaPlayerPlatformInterface->play(), "platformMediaPlayerPlayFailed" );
        
        m_expectedEventState = EventState::PLAYBACK_STARTED;
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"play").d("reason", ex.what()).d("expectedState",m_expectedEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::stop( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    try
    {
        ThrowIf( m_expectedEventState != EventState::NONE, "invalidEventState" );
        ThrowIfNot( validateSource( id ), "invalidSource" );
        
        m_expectedEventState = EventState::PLAYBACK_STOPPED;
        m_savedOffset = std::chrono::milliseconds( m_mediaPlayerPlatformInterface->getPosition() );

        ThrowIfNot( m_mediaPlayerPlatformInterface->stop(), "platformMediaPlayerStopFailed" );
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"stop").d("reason", ex.what()).d("expectedState",m_expectedEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::pause( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    try
    {
        ThrowIf( m_expectedEventState != EventState::NONE, "invalidEventState" );
        ThrowIfNot( validateSource( id ), "invalidSource" );

        m_expectedEventState = EventState::PLAYBACK_PAUSED;
        m_savedOffset = std::chrono::milliseconds( m_mediaPlayerPlatformInterface->getPosition() );

        ThrowIfNot( m_mediaPlayerPlatformInterface->pause(), "platformMediaPlayerPauseFailed" );
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"pause").d("reason", ex.what()).d("expectedState",m_expectedEventState));
        return false;
    }
}

bool AudioChannelEngineImpl::resume( alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id )
{
    try
    {
        ThrowIf( m_expectedEventState != EventState::NONE, "invalidEventState" );
        ThrowIfNot( validateSource( id ), "invalidSource" );

        m_expectedEventState = EventState::PLAYBACK_RESUMED;

        ThrowIfNot( m_mediaPlayerPlatformInterface->resume(), "platformMediaPlayerResumeFailed" );
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"resume").d("reason", ex.what()).d("expectedState",m_expectedEventState));
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
        ThrowIf( m_currentId == ERROR || m_currentId != id, "invalidSource" )
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"validateSource").d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::alexa
} // aace::engine
} // aace

