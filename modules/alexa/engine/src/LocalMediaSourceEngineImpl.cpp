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

#include "AACE/Engine/Alexa/LocalMediaSourceEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.LocalMediaSourceEngineImpl");

LocalMediaSourceEngineImpl::LocalMediaSourceEngineImpl( std::shared_ptr<aace::alexa::LocalMediaSource> platformLocalMediaSource, const std::string& localPlayerId, std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender, std::shared_ptr<FocusHandlerInterface> focusHandler ) :
    aace::engine::alexa::ExternalMediaAdapterHandler( platformLocalMediaSource->getSpeaker(), discoveredPlayerSender, focusHandler ),
    m_platformLocalMediaSource( platformLocalMediaSource ),
    m_localPlayerId( localPlayerId ) {
}

std::shared_ptr<LocalMediaSourceEngineImpl> LocalMediaSourceEngineImpl::create( std::shared_ptr<aace::alexa::LocalMediaSource> platformLocalMediaSource, const std::string& localPlayerId, std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender, std::shared_ptr<FocusHandlerInterface> focusHandler, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager )
{
    std::shared_ptr<LocalMediaSourceEngineImpl> localMediaSourceEngineImpl = nullptr;

    try
    {
        ThrowIfNull( platformLocalMediaSource, "invalidPlatformLocalMediaSource" );
        ThrowIf( localPlayerId.empty(), "invalidPlayerId" );
        ThrowIfNull( discoveredPlayerSender, "invalidDiscoveredPlayerSender" );
        ThrowIfNull( focusHandler, "invalidFocusHandler" );
        
        // create the external media adapter engine implementation
        localMediaSourceEngineImpl = std::shared_ptr<LocalMediaSourceEngineImpl>( new LocalMediaSourceEngineImpl( platformLocalMediaSource, localPlayerId, discoveredPlayerSender, focusHandler ) );
        ThrowIfNot( localMediaSourceEngineImpl->initialize( messageSender, speakerManager ), "initializeLocalMediaSourceEngineImplFailed" );

        // register the platforms engine interface
        platformLocalMediaSource->setEngineInterface( localMediaSourceEngineImpl );

        return localMediaSourceEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        if( localMediaSourceEngineImpl != nullptr ) {
            localMediaSourceEngineImpl->shutdown();
        }
        return nullptr;
    }
}

bool LocalMediaSourceEngineImpl::initialize( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager )
{
    try
    {
        ThrowIfNull( messageSender, "invalidMessageSender" );
        m_messageSender = messageSender;
        
        // initialize the adapter handler
        ThrowIfNot( initializeAdapterHandler( speakerManager ), "initializeAdapterHandlerFailed" );
        
        // report the player as discovered
        aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo info;
        
        info.localPlayerId = m_localPlayerId;
        info.spiVersion = "1.0";
        info.validationMethod = VALIDATION_NONE;
        info.validationData.push_back( "None" );
        
        reportDiscoveredPlayers( { info } );
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleAuthorization( const std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo>& authorizedPlayerList )
{
    try
    {
        // validate the authorized player list has exactly one player
        ThrowIf( authorizedPlayerList.size() != 1, "invalidAuthorizedPlayerList" );
        
        // call the authorize method in the platform implementation
        ThrowIfNot( m_platformLocalMediaSource->authorize( authorizedPlayerList[0].authorized ), "platformMediaAdapterAuthorizeFailed" );
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleAuthorization").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleLogin( const std::string& localPlayerId, const std::string& accessToken, const std::string& userName, bool forceLogin, std::chrono::milliseconds tokenRefreshInterval )
{
    try
    {
        Throw( "unsupportedLocalMediaSourceOperation" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleLogin").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleLogout( const std::string& localPlayerId )
{
    try
    {
        Throw( "unsupportedLocalMediaSourceOperation" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleLogout").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handlePlay( const std::string& localPlayerId, const std::string& playContextToken, int64_t index, std::chrono::milliseconds offset, bool preload, aace::alexa::ExternalMediaAdapter::Navigation navigation )
{
    try
    {
        ThrowIfNot( m_platformLocalMediaSource->play( "" ), "platformMediaAdapterPlayFailed" );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handlePlay").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handlePlayControl( const std::string& localPlayerId, aace::alexa::ExternalMediaAdapter::PlayControlType playControlType )
{
    try
    {
        ThrowIfNot( m_platformLocalMediaSource->playControl( playControlType ), "platformMediaAdapterPlayControlFailed" );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handlePlayControl").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleSeek( const std::string& localPlayerId, std::chrono::milliseconds offset )
{
    try
    {
        ThrowIfNot( m_platformLocalMediaSource->seek( offset ), "platformMediaAdapterSeekFailed" );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleSeek").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleAdjustSeek( const std::string& localPlayerId, std::chrono::milliseconds deltaOffset )
{
    try
    {
        ThrowIfNot( m_platformLocalMediaSource->adjustSeek( deltaOffset ), "platformMediaAdapterAdjustSeekFailed" );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"adjustSeek").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleGetAdapterState( const std::string& localPlayerId, alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::AdapterState& state )
{
    try
    {
        auto platformState = m_platformLocalMediaSource->getState();
        
        // session state
        if( platformState.sessionState.spiVersion.empty() == false ) {
            state.sessionState.spiVersion = platformState.sessionState.spiVersion;
        }

        state.sessionState.endpointId = platformState.sessionState.endpointId;
        state.sessionState.loggedIn = platformState.sessionState.loggedIn;
        state.sessionState.userName = platformState.sessionState.userName;
        state.sessionState.isGuest = platformState.sessionState.isGuest;
        state.sessionState.launched = platformState.sessionState.launched;
        state.sessionState.active = platformState.sessionState.active;
        state.sessionState.accessToken = platformState.sessionState.accessToken;
        state.sessionState.tokenRefreshInterval = platformState.sessionState.tokenRefreshInterval;
        state.sessionState.playerCookie = platformState.sessionState.playerCookie;
    
        // playback state
        state.playbackState.state = platformState.playbackState.state;
        state.playbackState.trackOffset = platformState.playbackState.trackOffset;
        state.playbackState.shuffleEnabled = platformState.playbackState.shuffleEnabled;
        state.playbackState.repeatEnabled = platformState.playbackState.repeatEnabled;
        state.playbackState.favorites = static_cast<alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::Favorites>( platformState.playbackState.favorites );
        state.playbackState.type = platformState.playbackState.type;
        state.playbackState.playbackSource = platformState.playbackState.playbackSource;
        state.playbackState.playbackSourceId = platformState.playbackState.playbackSourceId;
        state.playbackState.trackName = platformState.playbackState.trackName;
        state.playbackState.trackId = platformState.playbackState.trackId;
        state.playbackState.trackNumber = platformState.playbackState.trackNumber;
        state.playbackState.artistName = platformState.playbackState.artistName;
        state.playbackState.artistId = platformState.playbackState.artistId;
        state.playbackState.albumName = platformState.playbackState.albumName;
        state.playbackState.albumId = platformState.playbackState.albumId;
        state.playbackState.tinyURL = platformState.playbackState.tinyURL;
        state.playbackState.smallURL = platformState.playbackState.smallURL;
        state.playbackState.mediumURL = platformState.playbackState.mediumURL;
        state.playbackState.largeURL = platformState.playbackState.largeURL;
        state.playbackState.coverId = platformState.playbackState.coverId;
        state.playbackState.mediaProvider = platformState.playbackState.mediaProvider;
        state.playbackState.mediaType = static_cast<alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::MediaType>( platformState.playbackState.mediaType );
        state.playbackState.duration = platformState.playbackState.duration;
        
        for( auto nextOp : platformState.playbackState.supportedOperations ) {
            state.playbackState.supportedOperations.push_back( static_cast<alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::SupportedPlaybackOperation>( nextOp ) );
        }
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleGetAdapterState").d("reason", ex.what()));
        return false;
    }
}

// aace::alexa::LocalMediaSourceEngineInterface
void LocalMediaSourceEngineImpl::onPlayerEvent( const std::string& eventName )
{
    try
    {
        auto event = createExternalMediaPlayerEvent( m_localPlayerId, "PlayerEvent", true, [eventName](rapidjson::Value::Object& payload, rapidjson::Value::AllocatorType& allocator) {
            payload.AddMember( "eventName", rapidjson::Value().SetString( eventName.c_str(), eventName.length() ), allocator );
        });
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>( event );

        m_messageSender->sendMessage( request );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"onPlayerEvent").d("reason", ex.what()));
    }
}

void LocalMediaSourceEngineImpl::onPlayerError( const std::string& errorName, long code, const std::string& description, bool fatal )
{
    try
    {
        auto event = createExternalMediaPlayerEvent( m_localPlayerId, "PlayerError", true, [errorName,code,description,fatal](rapidjson::Value::Object& payload, rapidjson::Value::AllocatorType& allocator) {
            payload.AddMember( "errorName", rapidjson::Value().SetString( errorName.c_str(), errorName.length() ), allocator );
            payload.AddMember( "code", rapidjson::Value().SetInt64( code ), allocator );
            payload.AddMember( "description", rapidjson::Value().SetString( description.c_str(), description.length() ), allocator );
            payload.AddMember( "fatal", rapidjson::Value().SetBool( fatal ), allocator );
        });
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>( event );

        m_messageSender->sendMessage( request );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"onPlayerError").d("reason", ex.what()));
    }
}

void LocalMediaSourceEngineImpl::onSetFocus()
{
    try
    {
        ThrowIfNot( setFocus( m_localPlayerId ), "setFocusFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"onSetFocus").d("reason", ex.what()));
    }
}

// alexaClientSDK::avsCommon::utils::RequiresShutdown
void LocalMediaSourceEngineImpl::doShutdown()
{
    if( m_platformLocalMediaSource != nullptr ) {
        m_platformLocalMediaSource->setEngineInterface( nullptr );
        m_platformLocalMediaSource.reset();
    }

    m_messageSender.reset();
}

} // aace::engine::alexa
} // aace::engine
} // aace
