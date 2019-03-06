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

#ifndef AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_ENGINE_IMPL_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <functional>
#include <mutex>

#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/AVS/DirectiveHandlerConfiguration.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/ExternalMediaAdapterInterface.h>
#include <AVSCommon/SDKInterfaces/ExternalMediaPlayerInterface.h>
#include <AVSCommon/SDKInterfaces/ExternalMediaAdapterHandlerInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackHandlerInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackRouterInterface.h>
#include <AVSCommon/AVS/NamespaceAndName.h>
#include <AVSCommon/Utils/MediaPlayer/MediaPlayerInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <ExternalMediaPlayer/ExternalMediaPlayer.h>

#include "ExternalMediaAdapterHandler.h"

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/ExternalMediaAdapter.h"
#include "AACE/Alexa/LocalMediaSource.h"

#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace alexa {

class ExternalMediaPlayerEngineImpl :
    public DiscoveredPlayerSenderInterface,
    public FocusHandlerInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public alexaClientSDK::avsCommon::sdkInterfaces::ExternalMediaAdapterHandlerInterface,
    public alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface,
    public std::enable_shared_from_this<ExternalMediaPlayerEngineImpl> {
    
private:
    ExternalMediaPlayerEngineImpl( const std::string& agent );

    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter );
    
    std::shared_ptr<ExternalMediaAdapterHandler> getAdapter( const std::string& playerId );
    std::string createReportDiscoveredPlayersEvent();
    std::string createAuthorizationCompleteEvent();
    void updatePendingDiscoveredPlayers();
    std::string getLocalPlayerId( aace::alexa::LocalMediaSource::Source source );

public:
    static std::shared_ptr<ExternalMediaPlayerEngineImpl> create(
        const std::string& agent,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter );

    bool registerPlatformMediaAdapter( std::shared_ptr<aace::alexa::ExternalMediaAdapter> platformMediaAdapter );
    bool registerPlatformMediaAdapter( std::shared_ptr<aace::alexa::LocalMediaSource> platformMediaAdapter );
    
    // alexaClientSDK::avsCommon::sdkInterfaces::ExternalMediaAdapterHandlerInterface
    void authorizeDiscoveredPlayers( const std::string& payload ) override;
    void login( const std::string& payload ) override;
    void logout( const std::string& payload ) override;
    void play( const std::string& payload ) override;
    void playControl( const std::string& payload, alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::RequestType request ) override;
    void playControlForPlayer( const std::string& playerId, alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::RequestType request ) override;
    void seek( const std::string& payload ) override;
    void adjustSeek( const std::string& payload ) override;
    std::vector<alexaClientSDK::avsCommon::sdkInterfaces::externalMediaPlayer::AdapterState> getAdapterStates() override;
    
    // DiscoveredPlayerSenderInterface
    void reportDiscoveredPlayers( const std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>& discoveredPlayers ) override;
    void removeDiscoveredPlayer( const std::string& localPlayerId ) override;
    
    // FocusHandlerInterface
    void setFocus( const std::string& playerId ) override;

    // alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
    void onConnectionStatusChanged( const Status status, const ChangedReason reason ) override;

protected:
    virtual void doShutdown() override;

private:
    std::string m_agent;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> m_speakerManager;
    std::shared_ptr<alexaClientSDK::capabilityAgents::externalMediaPlayer::ExternalMediaPlayer> m_externalMediaPlayerCapabilityAgent;
    std::vector<std::shared_ptr<ExternalMediaAdapterHandler>> m_externalMediaAdapterList;
    std::unordered_map<std::string,std::shared_ptr<ExternalMediaAdapterHandler>> m_externalMediaAdapterMap;
    std::set<aace::alexa::LocalMediaSource::Source> m_registeredLocalMediaSourceAdapters;
    std::unordered_map<std::string,aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo> m_pendingDiscoveredPlayerMap;
    std::unordered_map<std::string,PlayerInfo> m_authorizationStateMap;
    Status m_connectionStatus = Status::DISCONNECTED;
    
    std::mutex m_discoveredPlayersMutex;
};

} // aace::engine::alexa
} // aace::engine
} // aace

#endif // AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_ENGINE_IMPL_H
