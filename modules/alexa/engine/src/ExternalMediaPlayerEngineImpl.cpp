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

#include <AVSCommon/Utils/UUIDGeneration/UUIDGeneration.h>
#include <AVSCommon/AVS/EventBuilder.h>

#include "AACE/Engine/Alexa/ExternalMediaPlayerEngineImpl.h"
#include "AACE/Engine/Alexa/ExternalMediaAdapterEngineImpl.h"
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
static const std::string TAG("aace.alexa.ExternalMediaPlayerEngineImpl");

static const std::string GLOBAL_PRESET_KEY = "preset";

ExternalMediaPlayerEngineImpl::ExternalMediaPlayerEngineImpl( const std::string& agent ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
    m_agent( agent ) {
}

bool ExternalMediaPlayerEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter ) {

    try
    {
        ThrowIfNull( directiveSequencer, "invalidDirectiveSequencer" );
        ThrowIfNull( capabilitiesDelegate, "invalidCapabilitiesDelegate" );
        ThrowIfNull( messageSender, "invalidMessageSender" );

        m_externalMediaPlayerCapabilityAgent = alexaClientSDK::capabilityAgents::externalMediaPlayer::ExternalMediaPlayer::create( {}, {}, {}, speakerManager, messageSender, focusManager, contextManager, exceptionSender, playbackRouter );
        ThrowIfNull( m_externalMediaPlayerCapabilityAgent, "couldNotCreateCapabilityAgent" );

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( m_externalMediaPlayerCapabilityAgent ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( capabilitiesDelegate->registerCapability( m_externalMediaPlayerCapabilityAgent ), "registerCapabilityFailed");

        // add ourself as a adapter handler in the external media player capability agent
        m_externalMediaPlayerCapabilityAgent->addAdapterHandler( std::dynamic_pointer_cast<alexaClientSDK::avsCommon::sdkInterfaces::ExternalMediaAdapterHandlerInterface>( shared_from_this() ) );

        m_messageSender = messageSender;
        m_speakerManager = speakerManager;

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<ExternalMediaPlayerEngineImpl> ExternalMediaPlayerEngineImpl::create(
    const std::string& agent,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter ) {

    std::shared_ptr<ExternalMediaPlayerEngineImpl> externalMediaPlayerEngineImpl = nullptr;

    try
    {
        // validate agent
        ThrowIf( agent.empty(), "invalidAgent" );
    
        // create the external media player impl
        externalMediaPlayerEngineImpl = std::shared_ptr<ExternalMediaPlayerEngineImpl>( new ExternalMediaPlayerEngineImpl( agent ) );
        ThrowIfNot( externalMediaPlayerEngineImpl->initialize( directiveSequencer, capabilitiesDelegate, speakerManager, messageSender, focusManager, contextManager, exceptionSender, playbackRouter ), "initializeExternalMediaPlayerEngineImplFailed" );

        return externalMediaPlayerEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        if( externalMediaPlayerEngineImpl != nullptr ) {
            externalMediaPlayerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

bool ExternalMediaPlayerEngineImpl::registerPlatformMediaAdapter( std::shared_ptr<aace::alexa::ExternalMediaAdapter> platformMediaAdapter )
{
    try
    {
        ThrowIfNull( platformMediaAdapter, "invalidExternalMediaAdapter" );
        
        // create the ExternalMediaAdapterEngineImpl instance
        auto externalMediaAdapterEngineImpl = ExternalMediaAdapterEngineImpl::create( platformMediaAdapter, shared_from_this(), shared_from_this(), m_messageSender.lock(), m_speakerManager.lock() );
        ThrowIfNull( externalMediaAdapterEngineImpl, "invalidExternalMediaAdapterEngineImpl" );
        
        // add the ExternalMediaAdapterEngineImpl to the ExternalMediaAdapterEngineImpl list
        m_externalMediaAdapterList.push_back( externalMediaAdapterEngineImpl );
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::registerPlatformMediaAdapter( std::shared_ptr<aace::alexa::LocalMediaSource> platformMediaAdapter )
{
    try
    {
        AACE_DEBUG(LX(TAG));

        ThrowIfNull( platformMediaAdapter, "invalidExternalMediaAdapter" );

        // get the source type from
        auto source = platformMediaAdapter->getSource();
        
        // check if the source is already registered with the media player
        ThrowIf( m_registeredLocalMediaSources.find( source ) != m_registeredLocalMediaSources.end(), "localMediaSourceAlreadyRegistered" );
        
        // get the local player id for the local media source type
        auto localPlayerId = getLocalPlayerIdForSource( source );
        ThrowIf( localPlayerId.empty(), "invalidLocalPlayerId" );

        // create the ExternalMediaAdapterEngineImpl instance
        auto localMediaSourceEngineImpl = LocalMediaSourceEngineImpl::create( platformMediaAdapter, localPlayerId, shared_from_this(), shared_from_this(), m_messageSender.lock(), m_speakerManager.lock() );
        ThrowIfNull( localMediaSourceEngineImpl, "invalidExternalMediaAdapterEngineImpl" );
        
        // add the ExternalMediaAdapterEngineImpl to the ExternalMediaAdapterEngineImpl list
        m_externalMediaAdapterList.push_back( localMediaSourceEngineImpl );
        m_registeredLocalMediaSources.emplace( source );

        return true;

    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::registerPlatformGlobalPresetHandler( std::shared_ptr<aace::alexa::GlobalPreset> globalPresetHandler )
{
    try
    {
        ThrowIfNull( globalPresetHandler, "invalidGlobalPresetHandler" );
        
        m_globalPresetHandler = globalPresetHandler;
    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<ExternalMediaAdapterHandler> ExternalMediaPlayerEngineImpl::getAdapter( const std::string& playerId ) 
{
    auto it = m_externalMediaAdapterMap.find( playerId );
    return it != m_externalMediaAdapterMap.end() ? it->second : nullptr;
}

// alexaClientSDK::avsCommon::sdkInterfaces::ExternalMediaAdapterHandlerInterface

void ExternalMediaPlayerEngineImpl::authorizeDiscoveredPlayers( const std::string& payload )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        std::vector<PlayerInfo> acknowledgedPlayerList;
        rapidjson::Document document;
        
        // parse the document
        document.Parse( payload.c_str() );
        
        // check the parser error
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidPayload" );

        // get the root payload object
        auto root = document.GetObject();

        // get the authorized players list from the payload
        ThrowIfNot( root.HasMember( "players" ) && root["players"].IsArray(), "invalidPlayers" );
        auto players = root["players"].GetArray();
        
        std::unique_lock<std::mutex> lock( m_playersMutex );

        // generate the platform authorized player list
        for( unsigned int j = 0; j < players.Size(); j++ )
        {
            try {
                auto next = players[j].GetObject();
                
                PlayerInfo acknowledgedPlayerInfo;

                ThrowIfNot( next.HasMember( "localPlayerId" ) && next["localPlayerId"].IsString(), "invalidLocalPlayerId" );
                std::string localPlayerId = next["localPlayerId"].GetString();
                acknowledgedPlayerInfo.localPlayerId = localPlayerId;

                if( m_pendingDiscoveredPlayerMap.find( localPlayerId ) != m_pendingDiscoveredPlayerMap.end() ) {
                    // Player has been acknowledged by cloud or local skill. Safe to exclude from future
                    // ReportDiscoveredPlayers events
                    AACE_VERBOSE(LX(TAG).d("removingPlayerFromPendingDiscoveryList", localPlayerId));
                    m_pendingDiscoveredPlayerMap.erase( localPlayerId );
                }
                
                // set the authorized player info
                ThrowIfNot( next.HasMember( "authorized" ) && next["authorized"].IsBool(), "invalidAuthorized" );
                acknowledgedPlayerInfo.authorized = next["authorized"].GetBool();
                
                if( acknowledgedPlayerInfo.authorized )
                {
                    ThrowIfNot( next.HasMember( "metadata" ) && next["metadata"].IsObject(), "invalidMetadata" );
                    auto metadata = next["metadata"].GetObject();
                
                    ThrowIfNot( metadata.HasMember( "playerId" ) && metadata["playerId"].IsString(), "invalidPlayerId" );
                    acknowledgedPlayerInfo.playerId = metadata["playerId"].GetString();
                
                    ThrowIfNot( metadata.HasMember( "skillToken" ) && metadata["skillToken"].IsString(), "invalidSkillToken" );
                    acknowledgedPlayerInfo.skillToken = metadata["skillToken"].GetString();
                }
                
                // add the player info to the authorized player list
                acknowledgedPlayerList.push_back( acknowledgedPlayerInfo );
                
                // add the player info to the authorized player map
                m_authorizationStateMap[acknowledgedPlayerInfo.localPlayerId] = acknowledgedPlayerInfo;
            }
            catch( std::exception& ex ) {
                AACE_ERROR(LX(TAG).d("reason", ex.what()));
            }
        }
        
        // call the platform media adapter authorization method
        if( acknowledgedPlayerList.empty() == false )
        {
            for( auto& nextAdapter: m_externalMediaAdapterList )
            {
                auto acknowledgedPlayers = nextAdapter->authorizeDiscoveredPlayers( acknowledgedPlayerList );
         
                // add the authorized players to the media adapter player id map
                for( auto& nextPlayerInfo : acknowledgedPlayers ) {
                    if( nextPlayerInfo.authorized ) {
                        m_externalMediaAdapterMap[nextPlayerInfo.playerId] = nextAdapter;
                    }
                }
            }
        }
        
        // send the authorization complete event
        auto event = createAuthorizationCompleteEventLocked();
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>( event );
        lock.unlock();

        AACE_VERBOSE(LX(TAG).m("sendingAuthorizationCompleteEvent"));
        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull( m_messageSender_lock, "invalidMessageSender" );
        m_messageSender_lock->sendMessage( request );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ExternalMediaPlayerEngineImpl::login( const std::string& payload )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document;
        
        // parse the document
        document.Parse( payload.c_str() );
        
        // check the parser error
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidPayload" );
        
        // get the root payload object
        auto root = document.GetObject();
        
        ThrowIfNot( root.HasMember( "playerId" ) && root["playerId"].IsString(), "invalidPlayerId" );
        auto playerId = root["playerId"].GetString();
        
        ThrowIfNot( root.HasMember( "accessToken" ) && root["accessToken"].IsString(), "invalidAccessToken" );
        auto accessToken = root["accessToken"].GetString();
        
        ThrowIfNot( root.HasMember( "username" ) && root["username"].IsString(), "invalidUsername" );
        auto username = root["username"].GetString();
        
        ThrowIfNot( root.HasMember( "tokenRefreshIntervalInMilliseconds" ) && root["tokenRefreshIntervalInMilliseconds"].IsInt64(), "invalidTokenRefreshInterval" );
        auto tokenRefreshIntervalInMilliseconds = root["tokenRefreshIntervalInMilliseconds"].GetInt64();
        
        ThrowIfNot( root.HasMember( "forceLogin" ) && root["forceLogin"].IsBool(), "invalidForceLogin" );
        auto forceLogin = root["forceLogin"].GetBool();
        
        auto adapter = getAdapter( playerId );
        ThrowIfNull( adapter, "invalidMediaPlayerAdapter" );
        
        // call the adapter method
        ThrowIfNot( adapter->login( playerId, accessToken, username, forceLogin, std::chrono::milliseconds( tokenRefreshIntervalInMilliseconds ) ), "adapterLoginFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ExternalMediaPlayerEngineImpl::logout( const std::string& payload )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document;
        
        // parse the document
        document.Parse( payload.c_str() );
        
        // check the parser error
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidPayload" );
        
        // get the root payload object
        auto root = document.GetObject();
        
        ThrowIfNot( root.HasMember( "playerId" ) && root["playerId"].IsString(), "invalidPlayerId" );
        auto playerId = root["playerId"].GetString();
        
        auto adapter = getAdapter( playerId );
        ThrowIfNull( adapter, "invalidMediaPlayerAdapter" );
        
        // call the adapter method
        ThrowIfNot( adapter->logout( playerId ), "adapterLogoutFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

static const std::unordered_map<std::string,aace::alexa::ExternalMediaAdapter::Navigation> NAVIGATION_ENUM_MAP = {
    { "DEFAULT", aace::alexa::ExternalMediaAdapter::Navigation::DEFAULT },
    { "NONE", aace::alexa::ExternalMediaAdapter::Navigation::NONE },
    { "FOREGROUND", aace::alexa::ExternalMediaAdapter::Navigation::FOREGROUND }
};

static aace::alexa::ExternalMediaAdapter::Navigation getNavigationEnum( std::string name )
{
    std::transform( name.begin(), name.end(), name.begin(), [](unsigned char c) -> unsigned char { return static_cast<unsigned char>(std::toupper(c)); } );
    
    auto it = NAVIGATION_ENUM_MAP.find( name );
    
    return it != NAVIGATION_ENUM_MAP.end() ? it->second : aace::alexa::ExternalMediaAdapter::Navigation::DEFAULT;
}

void ExternalMediaPlayerEngineImpl::play( const std::string& payload )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document;
        
        // parse the document
        document.Parse( payload.c_str() );
        
        // check the parser error
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidPayload" );
        
        // get the root payload object
        auto root = document.GetObject();
        
        ThrowIfNot( root.HasMember( "playerId" ) && root["playerId"].IsString(), "invalidPlayerId" );
        auto playerId = root["playerId"].GetString();
        
        ThrowIfNot( root.HasMember( "skillToken" ) && root["skillToken"].IsString(), "invalidSkillToken" );
        auto skillToken = root["skillToken"].GetString();

        ThrowIfNot( root.HasMember( "playbackSessionId" ) && root["playbackSessionId"].IsString(), "invalidPlaybackSessionId" );
        auto playbackSessionId = root["playbackSessionId"].GetString();

        ThrowIfNot( root.HasMember( "index" ) && root["index"].IsInt64(), "invalidIndex" );
        auto index = root["index"].GetInt64();

        ThrowIfNot( root.HasMember( "offsetInMilliseconds" ) && root["offsetInMilliseconds"].IsInt64(), "invalidOffsetInMilliseconds" );
        auto offsetInMilliseconds = root["offsetInMilliseconds"].GetInt64();

        ThrowIfNot( root.HasMember( "playbackContextToken" ) && root["playbackContextToken"].IsString(), "invalidPlaybackContextToken" );
        auto playbackContextToken = root["playbackContextToken"].GetString();
        
        ThrowIfNot( root.HasMember( "preload" ) && root["preload"].IsBool(), "invalidPreload" );
        auto preload = root["preload"].GetBool();

        ThrowIfNot( root.HasMember( "navigation" ) && root["navigation"].IsString(), "invalidNavigation" );
        auto navigation = root["navigation"].GetString();
        
        if ( ( playerId != NULL ) && ( playerId[0] == '\0' ) ) {// empty playerId == global preset case
            std::string token = playbackContextToken;
            size_t gpindex = token.find(GLOBAL_PRESET_KEY);
            ThrowIfNot( gpindex != std::string::npos , "invalidPayloadWithGlobalPreset" );
            std::string presetString = token.substr(gpindex + GLOBAL_PRESET_KEY.length() + 1); //  from "preset:" to end
            int preset = std::stoi( presetString );
            ThrowIfNull( m_globalPresetHandler, "platformGlobalPresetHandlerNull" );
            // send global preset to platform
            m_globalPresetHandler->setGlobalPreset( preset );
        } else {
            // get the platform adapter
            auto adapter = getAdapter( playerId );
            ThrowIfNull( adapter, "invalidMediaPlayerAdapter" );
            
            // call the adapter method
            ThrowIfNot( adapter->play( playerId, playbackContextToken, index, std::chrono::milliseconds( offsetInMilliseconds ), skillToken, playbackSessionId, preload, getNavigationEnum( navigation ) ), "adapterPlayFailed" );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ExternalMediaPlayerEngineImpl::playControl( const std::string& payload, alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::RequestType request )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document;
        
        // parse the document
        document.Parse( payload.c_str() );
        
        // check the parser error
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidPayload" );
        
        // get the root payload object
        auto root = document.GetObject();
        
        ThrowIfNot( root.HasMember( "playerId" ) && root["playerId"].IsString(), "invalidPlayerId" );
        auto playerId = root["playerId"].GetString();
        
        playControlForPlayer( playerId, request );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ExternalMediaPlayerEngineImpl::playControlForPlayer( const std::string& playerId, alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::RequestType request )
{
    try
    {
        AACE_VERBOSE(LX(TAG).d("playerId",playerId));

        // get the platform adapter
        auto adapter = getAdapter( playerId );
        ThrowIfNull( adapter, "invalidMediaPlayerAdapter" );

        // call the adapter method
        ThrowIfNot( adapter->playControl( playerId, request ), "adapterPlayControlFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ExternalMediaPlayerEngineImpl::seek( const std::string& payload )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document;
        
        // parse the document
        document.Parse( payload.c_str() );
        
        // check the parser error
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidPayload" );
        
        // get the root payload object
        auto root = document.GetObject();
        
        ThrowIfNot( root.HasMember( "playerId" ) && root["playerId"].IsString(), "invalidPlayerId" );
        auto playerId = root["playerId"].GetString();
        
        ThrowIfNot( root.HasMember( "positionMilliseconds" ) && root["positionMilliseconds"].IsInt64(), "invalidPositionMilliseconds" );
        auto positionMilliseconds = root["positionMilliseconds"].GetInt64();
        
        // get the platform adapter
        auto adapter = getAdapter( playerId );
        ThrowIfNull( adapter, "invalidMediaPlayerAdapter" );

        // call the adapter method
        ThrowIfNot( adapter->seek( playerId, std::chrono::milliseconds( positionMilliseconds ) ), "adapterSeekFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ExternalMediaPlayerEngineImpl::adjustSeek( const std::string& payload )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document;
        
        // parse the document
        document.Parse( payload.c_str() );
        
        // check the parser error
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidPayload" );
        
        // get the root payload object
        auto root = document.GetObject();
        
        ThrowIfNot( root.HasMember( "playerId" ) && root["playerId"].IsString(), "invalidPlayerId" );
        auto playerId = root["playerId"].GetString();
        
        ThrowIfNot( root.HasMember( "deltaPositionMilliseconds" ) && root["deltaPositionMilliseconds"].IsInt64(), "invalidDeltaPositionMilliseconds" );
        auto deltaPositionMilliseconds = root["deltaPositionMilliseconds"].GetInt64();
        
        // get the platform adapter
        auto adapter = getAdapter( playerId );
        ThrowIfNull( adapter, "invalidMediaPlayerAdapter" );

        // call the adapter method
        ThrowIfNot( adapter->adjustSeek( playerId, std::chrono::milliseconds( deltaPositionMilliseconds ) ), "adapterAdjustSeekFailed" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::vector<alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::AdapterState> ExternalMediaPlayerEngineImpl::getAdapterStates()
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        std::vector<alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::AdapterState> adapterStateList;

        // iterate through the media adapter list and add all of the adapter states
        // for the players that the adapter handles...
        for( auto next : m_externalMediaAdapterList ) {
            auto adapterStates = next->getAdapterStates();
            adapterStateList.insert( adapterStateList.end(), adapterStates.begin(), adapterStates.end() );
        }
        
        return adapterStateList;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::vector<alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::AdapterState>();
    }
}

std::string ExternalMediaPlayerEngineImpl::createReportDiscoveredPlayersEventLocked( const DiscoveredPlayerMap& discoveredPlayers )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document( rapidjson::kObjectType );
        
        // create payload data
        auto payload = document.GetObject();

        // add agent
        payload.AddMember( "agent", rapidjson::Value().SetString( m_agent.c_str(), m_agent.length(), document.GetAllocator() ), document.GetAllocator() );
        
        // add players
        rapidjson::Value playerList( rapidjson::kArrayType );
        
        for( const auto& next : discoveredPlayers )
        {
            auto info = next.second;
        
            rapidjson::Value player( rapidjson::kObjectType );
            
            player.AddMember( "localPlayerId", rapidjson::Value().SetString( info.localPlayerId.c_str(), info.localPlayerId.length(), document.GetAllocator()), document.GetAllocator() );
            player.AddMember( "spiVersion", rapidjson::Value().SetString( info.spiVersion.c_str(), info.spiVersion.length(), document.GetAllocator()), document.GetAllocator() );
            player.AddMember( "validationMethod", rapidjson::Value().SetString( info.validationMethod.c_str(), info.validationMethod.length(), document.GetAllocator()), document.GetAllocator() );
            
            // create the validation data
            rapidjson::Value validationData( rapidjson::kArrayType );
            
            for( auto& nextValidationData : info.validationData ) {
                validationData.PushBack( rapidjson::Value().SetString( nextValidationData.c_str(), nextValidationData.length(), document.GetAllocator()), document.GetAllocator() );
            }
            
            player.AddMember( "validationData", validationData, document.GetAllocator() );

            // add the player to the playerList array
            playerList.PushBack( rapidjson::Value().CopyFrom( player, document.GetAllocator() ), document.GetAllocator() );
        }
        
        // add the player list to the payload
        payload.AddMember( "players", playerList, document.GetAllocator() );
        
        // create payload string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );

        document.Accept( writer );
    
        return alexaClientSDK::avsCommon::avs::buildJsonEventString( "ExternalMediaPlayer", "ReportDiscoveredPlayers", "", buffer.GetString() ).second;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

std::string ExternalMediaPlayerEngineImpl::createAuthorizationCompleteEventLocked()
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document( rapidjson::kObjectType );
        
        // create payload data
        auto payload = document.GetObject();

        rapidjson::Value authorizedList( rapidjson::kArrayType );
        rapidjson::Value deauthorizedList( rapidjson::kArrayType );

        // add authorized and deauthorized lists
        for( const auto& next : m_authorizationStateMap )
        {
            auto info = next.second;
            
            rapidjson::Value player( rapidjson::kObjectType );

            if( info.authorized )
            {
                player.AddMember( "playerId", rapidjson::Value().SetString( info.playerId.c_str(), info.playerId.length(), document.GetAllocator() ), document.GetAllocator() );
                player.AddMember( "skillToken", rapidjson::Value().SetString( info.skillToken.c_str(), info.skillToken.length(), document.GetAllocator() ), document.GetAllocator() );
                authorizedList.PushBack( rapidjson::Value().CopyFrom( player, document.GetAllocator() ), document.GetAllocator() );
            }
            else
            {
                player.AddMember( "localPlayerId", rapidjson::Value().SetString( info.localPlayerId.c_str(), info.localPlayerId.length(), document.GetAllocator() ), document.GetAllocator() );
                deauthorizedList.PushBack( rapidjson::Value().CopyFrom( player, document.GetAllocator() ), document.GetAllocator() );
            }
        }
        
        payload.AddMember( "authorized", authorizedList, document.GetAllocator() );
        payload.AddMember( "deauthorized", deauthorizedList, document.GetAllocator() );

        // create payload string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );

        document.Accept( writer );

        return alexaClientSDK::avsCommon::avs::buildJsonEventString( "ExternalMediaPlayer", "AuthorizationComplete", "", buffer.GetString() ).second;
    
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

// DiscoveredPlayerSenderInterface
void ExternalMediaPlayerEngineImpl::reportDiscoveredPlayers( const std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>& discoveredPlayers )
{
    AACE_VERBOSE(LX(TAG));
    m_executor.submit([this, discoveredPlayers]() {
        try
        {
            AACE_VERBOSE(LX(TAG,"reportDiscoveredPlayersExec"));

            std::lock_guard<std::mutex> lock( m_playersMutex );

            for( auto& next : discoveredPlayers )
            {
                if( m_pendingDiscoveredPlayerMap.find( next.localPlayerId ) == m_pendingDiscoveredPlayerMap.end() )
                {
                    AACE_VERBOSE(LX(TAG).m("addingDiscoveredPlayerToPendingMap").d("localPlayerId",next.localPlayerId));
                    m_pendingDiscoveredPlayerMap[next.localPlayerId] = next;
                }
                else {
                    AACE_WARN(LX(TAG).d("reason","discoveredPlayerAlreadyPending").d("localPlayerId",next.localPlayerId));
                }
            }
            
            // send the pending discovered players if possible
            sendDiscoveredPlayersIfReadyLocked( m_pendingDiscoveredPlayerMap );

        }
        catch( std::exception& ex ) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    });
}

void ExternalMediaPlayerEngineImpl::removeDiscoveredPlayer( const std::string& localPlayerId ) 
{
    AACE_VERBOSE(LX(TAG).d("removingPlayerId",localPlayerId));
    m_executor.submit([this, localPlayerId]() {
        std::lock_guard<std::mutex> lock( m_playersMutex );
        m_authorizationStateMap.erase( localPlayerId );
    });
}

void ExternalMediaPlayerEngineImpl::sendDiscoveredPlayersIfReadyLocked( const DiscoveredPlayerMap& discoveredPlayers )
{
    try
    {
        AACE_VERBOSE(LX(TAG));

        std::unique_lock<std::mutex> lock( m_connectionMutex );
        auto connectionStatus = m_connectionStatus;
        lock.unlock();
        if( connectionStatus == Status::CONNECTED && discoveredPlayers.empty() == false )
        {
            auto event = createReportDiscoveredPlayersEventLocked( discoveredPlayers );
            auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>( event );
        
            AACE_VERBOSE(LX(TAG).m("sendingReportDiscoveredPlayersEvent"));
            
            auto m_messageSender_lock = m_messageSender.lock();
            ThrowIfNull( m_messageSender_lock, "invalidMessageSender"  );
            m_messageSender_lock->sendMessage( request );
            
            // Note: we wait to clear the pending discovery map until the players
            // are confirmed by an AuthorizeDiscoveredPlayers directive
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string ExternalMediaPlayerEngineImpl::getLocalPlayerIdForSource( aace::alexa::LocalMediaSource::Source source )
{
    try
    {
        switch( source )
        {
            case aace::alexa::LocalMediaSource::Source::AM_RADIO:
                return "com.amazon.alexa.auto.players.AM_RADIO";
                
            case aace::alexa::LocalMediaSource::Source::FM_RADIO:
                return "com.amazon.alexa.auto.players.FM_RADIO";
                
            case aace::alexa::LocalMediaSource::Source::BLUETOOTH:
                return "com.amazon.alexa.auto.players.BLUETOOTH";
                
            case aace::alexa::LocalMediaSource::Source::COMPACT_DISC:
                return "com.amazon.alexa.auto.players.COMPACT_DISC";
                
            case aace::alexa::LocalMediaSource::Source::LINE_IN:
                return "com.amazon.alexa.auto.players.LINE_IN";
                
            case aace::alexa::LocalMediaSource::Source::SATELLITE_RADIO:
                return "com.amazon.alexa.auto.players.SATELLITE_RADIO";
        
            case aace::alexa::LocalMediaSource::Source::USB:
                return "com.amazon.alexa.auto.players.USB";
            
            case aace::alexa::LocalMediaSource::Source::SIRIUS_XM:
                return "com.amazon.alexa.auto.players.SIRIUS_XM";

            case aace::alexa::LocalMediaSource::Source::DAB:
                return "com.amazon.alexa.auto.players.DAB";
        
            default:
                throw( "invalidLocalMediaSource" );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("source",source));
        return "";
    }
}

// FocusHandlerInterface
void ExternalMediaPlayerEngineImpl::setFocus( const std::string& playerId ) {
    AACE_VERBOSE(LX(TAG).d("playerId",playerId));
    m_externalMediaPlayerCapabilityAgent->setPlayerInFocus( playerId, true );
}

// alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
void ExternalMediaPlayerEngineImpl::onConnectionStatusChanged( const Status status, const ChangedReason reason )
{
    AACE_VERBOSE(LX(TAG).d("status",status));

    std::lock_guard<std::mutex> lock( m_connectionMutex );
    if( m_connectionStatus != status ) {
        m_connectionStatus = status;
        m_executor.submit([this]() {
            std::lock_guard<std::mutex> lock( m_playersMutex );
            sendDiscoveredPlayersIfReadyLocked( m_pendingDiscoveredPlayerMap );
        });
    }
}

// alexaClientSDK::avsCommon::utils::RequiresShutdown
void ExternalMediaPlayerEngineImpl::doShutdown()
{
    // shut down external media adapters
    for( auto& adapter : m_externalMediaAdapterList ) {
        adapter->shutdown();
    }
    // clear the external media adapter list
    m_externalMediaAdapterList.clear();

    // clear the media adapter player id map
    m_externalMediaAdapterMap.clear();

    // shutdown the capability agent
    if( m_externalMediaPlayerCapabilityAgent != nullptr ) {
        m_externalMediaPlayerCapabilityAgent->shutdown();
        m_externalMediaPlayerCapabilityAgent.reset();
    }
}

} // aace::engine::alexa
} // aace::engine
} // aace
