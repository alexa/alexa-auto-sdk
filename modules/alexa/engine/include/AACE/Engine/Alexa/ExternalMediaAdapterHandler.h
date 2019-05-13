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

#ifndef AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_HANDLER_H
#define AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_HANDLER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <functional>

#include <AVSCommon/SDKInterfaces/ExternalMediaAdapterInterface.h>
#include <AVSCommon/SDKInterfaces/ExternalMediaPlayerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/ExternalMediaAdapter.h"

#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace alexa {

class DiscoveredPlayerSenderInterface;
class FocusHandlerInterface;

static const std::string VALIDATION_SIGNING_CERTIFICATE = "SIGNING_CERTIFICATE";
static const std::string VALIDATION_GENERATED_CERTIFICATE = "GENERATED_CERTIFICATE";
static const std::string VALIDATION_NONE = "NONE";

class PlayerInfo {
public:
    PlayerInfo( const std::string& localId = "", const std::string& spi = "", bool authorized = false );

public:
    std::string localPlayerId;
    std::string spiVersion;
    std::string playerId;
    std::string skillToken;
    std::string playbackSessionId;
    bool authorized;
};

class ExternalMediaAdapterHandler :
    public aace::alexa::SpeakerEngineInterface,
    public alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<ExternalMediaAdapterHandler> {
    
protected:
    ExternalMediaAdapterHandler( std::shared_ptr<aace::alexa::Speaker> speakerPlatformInterface, std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender, std::shared_ptr<FocusHandlerInterface> focusHandler );
    
    bool initializeAdapterHandler( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager );

    bool validatePlayer( const std::string& localPlayerId, bool checkAuthorized = true );
    bool setFocus( const std::string& localPlayerId );

    std::string createExternalMediaPlayerEvent( const std::string& localPlayerId, const std::string& event, bool includePlaybackSessionId = false, std::function<void(rapidjson::Value::Object&,rapidjson::Value::AllocatorType&)> createPayload = [](rapidjson::Value::Object& v,rapidjson::Value::AllocatorType& a) {} );
    
    void reportDiscoveredPlayers( const std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>& discoveredPlayers );
    bool removeDiscoveredPlayer( const std::string& localPlayerId );

    // ExternalMediaAdapterHandler interface
    virtual bool handleAuthorization( const std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo>& authorizedPlayerList ) = 0;
    virtual bool handleLogin( const std::string& playerId, const std::string& accessToken, const std::string& userName, bool forceLogin, std::chrono::milliseconds tokenRefreshInterval ) = 0;
    virtual bool handleLogout( const std::string& playerId ) = 0;
    virtual bool handlePlay( const std::string& playerId, const std::string& playContextToken, int64_t index, std::chrono::milliseconds offset, bool preload, aace::alexa::ExternalMediaAdapter::Navigation navigation ) = 0;
    virtual bool handlePlayControl( const std::string& playerId, aace::alexa::ExternalMediaAdapter::PlayControlType playControlType ) = 0;
    virtual bool handleSeek( const std::string& playerId, std::chrono::milliseconds offset ) = 0;
    virtual bool handleAdjustSeek( const std::string& playerId, std::chrono::milliseconds deltaOffset ) = 0;
    virtual bool handleGetAdapterState( const std::string& playerId, alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::AdapterState& state ) = 0;

    // alexaClientSDK::avsCommon::utils::RequiresShutdown
    virtual void doShutdown() override = 0;

public:
    std::vector<PlayerInfo> authorizeDiscoveredPlayers( const std::vector<PlayerInfo>& authorizedPlayerList );
    bool login( const std::string& playerId, const std::string& accessToken, const std::string& userName, bool forceLogin, std::chrono::milliseconds tokenRefreshInterval );
    bool logout( const std::string& playerId );
    bool play( const std::string& playerId, const std::string& playContextToken, int64_t index, std::chrono::milliseconds offset, const std::string& skillToken, const std::string& playbackSessionId, bool preload, aace::alexa::ExternalMediaAdapter::Navigation navigation );
    bool playControl( const std::string& playerId, alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::RequestType requestType );
    bool seek( const std::string& playerId, std::chrono::milliseconds offset );
    bool adjustSeek( const std::string& playerId, std::chrono::milliseconds deltaOffset );
    std::vector<alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::AdapterState> getAdapterStates();

    // aace::engine::SpeakerEngineInterface
    void onLocalVolumeSet( int8_t volume ) override;
    void onLocalMuteSet( bool mute ) override;

    // alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface
    bool setVolume( int8_t volume ) override;
    bool adjustVolume( int8_t delta ) override;
    bool setMute( bool mute ) override;
    bool getSpeakerSettings( alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings ) override;
    alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type getSpeakerType() override;

private:
    std::shared_ptr<aace::alexa::Speaker> m_speakerPlatformInterface;
    std::shared_ptr<DiscoveredPlayerSenderInterface> m_discoveredPlayerSender;
    std::shared_ptr<FocusHandlerInterface> m_focusHandler;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> m_speakerManager;
    
    std::unordered_map<std::string,PlayerInfo> m_playerInfoMap;
    std::unordered_map<std::string,std::string> m_alexaToLocalPlayerIdMap;
};

class DiscoveredPlayerSenderInterface {
public:
    virtual void reportDiscoveredPlayers( const std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>& discoveredPlayers ) = 0;
    virtual void removeDiscoveredPlayer( const std::string& localPlayerId ) = 0;
};

class FocusHandlerInterface {
public:
    virtual void setFocus( const std::string& playerId ) = 0;
};

} // aace::engine::alexa
} // aace::engine
} // aace

#endif // AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_ADAPTER_HANDLER_H
