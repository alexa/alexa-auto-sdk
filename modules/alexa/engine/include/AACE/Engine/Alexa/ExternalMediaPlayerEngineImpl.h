/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/AVS/DirectiveHandlerConfiguration.h>
#include <AVSCommon/AVS/NamespaceAndName.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/RenderPlayerInfoCardsProviderInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
// #include <AVSCommon/SDKInterfaces/ExternalMediaAdapterInterface.h>
// #include <AVSCommon/SDKInterfaces/ExternalMediaAdapterHandlerInterface.h>
// #include <AVSCommon/SDKInterfaces/ExternalMediaPlayerInterface.h>
#include "ExternalMediaAdapterInterface.h"
#include "ExternalMediaAdapterHandlerInterface.h"
#include "ExternalMediaPlayerInterface.h"
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackHandlerInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackRouterInterface.h>
#include <AVSCommon/Utils/MediaPlayer/MediaPlayerInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <Endpoints/EndpointBuilder.h>
// #include <ExternalMediaPlayer/ExternalMediaPlayer.h>
#include "ExternalMediaPlayer.h"

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/ExternalMediaAdapter.h"
#include "AACE/Alexa/GlobalPreset.h"
#include "AACE/Alexa/LocalMediaSource.h"
#include "AACE/Engine/Alexa/ExternalMediaAdapterHandler.h"
#include "AACE/Engine/Network/NetworkInfoObserver.h"
#include "AACE/Engine/Network/NetworkObservableInterface.h"

#include <rapidjson/document.h>

namespace aace {
namespace engine {
namespace alexa {

class AudioPlayerObserverDelegate;

class ExternalMediaPlayerEngineImpl
        : public DiscoveredPlayerSenderInterface
        , public FocusHandlerInterface
        , public aace::engine::alexa::ExternalMediaAdapterHandlerInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface
        , public std::enable_shared_from_this<ExternalMediaPlayerEngineImpl> {
private:
    using DiscoveredPlayerMap =
        std::unordered_map<std::string, aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>;

    ExternalMediaPlayerEngineImpl(const std::string& agent);

    // functions without the "Locked" suffix must not be called when the calling thread already holds @c m_playersMutex

    bool initialize(
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedMessageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
        std::shared_ptr<aace::engine::alexa::AudioPlayerObserverDelegate> audioPlayerObserverDelegate,
        std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface>
            externalMediaAdapterRegistration);

    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterHandlerInterface> getAdapter(const std::string& playerId);
    std::string getLocalPlayerIdForSource(aace::alexa::LocalMediaSource::Source source);

    // functions with the "Locked" suffix must only be called if the calling thread holds @c m_playersMutex

    void sendDiscoveredPlayersIfReadyLocked(const DiscoveredPlayerMap& discoveredPlayers);
    std::string createReportDiscoveredPlayersEventLocked(const DiscoveredPlayerMap& discoveredPlayers);
    std::string createAuthorizationCompleteEventLocked();

public:
    static std::shared_ptr<ExternalMediaPlayerEngineImpl> create(
        const std::string& agent,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedMessageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
        std::shared_ptr<aace::engine::alexa::AudioPlayerObserverDelegate> audioPlayerObserverDelegate,
        std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface>
            externalMediaAdapterRegistration);

    std::shared_ptr<aace::engine::alexa::ExternalMediaPlayer> getExternalMediaPlayerCapabilityAgent();

    bool registerPlatformMediaAdapter(std::shared_ptr<aace::alexa::ExternalMediaAdapter> platformMediaAdapter);
    bool registerPlatformMediaAdapter(std::shared_ptr<aace::alexa::LocalMediaSource> platformMediaAdapter);
    bool registerPlatformGlobalPresetHandler(std::shared_ptr<aace::alexa::GlobalPreset> platformGlobalPreset);

    // alexaClientSDK::avsCommon::sdkInterfaces::ExternalMediaAdapterHandlerInterface
    std::vector<aace::engine::alexa::PlayerInfo> authorizeDiscoveredPlayers(
        const std::vector<aace::engine::alexa::PlayerInfo>& authorizedPlayerList) override;
    bool login(
        const std::string& playerId,
        const std::string& accessToken,
        const std::string& userName,
        bool forceLogin,
        std::chrono::milliseconds tokenRefreshInterval) override;
    bool logout(const std::string& playerId) override;
    bool play(
        const std::string& playerId,
        const std::string& playContextToken,
        const int64_t index,
        const std::chrono::milliseconds offset,
        const std::string& skillToken,
        const std::string& playbackSessionId,
        const std::string& navigation,
        const bool preload,
        const alexaClientSDK::avsCommon::avs::PlayRequestor& playRequestor) override;
    bool playControl(const std::string& playerId, aace::engine::alexa::RequestType requestType) override;
    bool seek(const std::string& playerId, std::chrono::milliseconds positionMilliseconds) override;
    bool adjustSeek(const std::string& playerId, std::chrono::milliseconds offsetMilliseconds) override;
    std::vector<aace::engine::alexa::AdapterState> getAdapterStates(bool all) override;
    std::chrono::milliseconds getOffset(const std::string& playerId) override;

    // DiscoveredPlayerSenderInterface
    void reportDiscoveredPlayers(
        const std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>& discoveredPlayers) override;
    void removeDiscoveredPlayer(const std::string& localPlayerId) override;

    // FocusHandlerInterface
    void setFocus(const std::string& playerId, bool focusAcquire) override;

    // alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
    void onConnectionStatusChanged(const Status status, const ChangedReason reason) override;

    // RenderPlayerInfoCardsObserver interface
    void setObserver(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface>
                         observer) override;

protected:
    virtual void doShutdown() override;

private:
    std::string m_agent;

    std::weak_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;
    std::weak_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> m_speakerManager;

    std::shared_ptr<aace::engine::alexa::ExternalMediaPlayer> m_externalMediaPlayerCapabilityAgent;
    /**
     * A list of every registered @c ExternalMediaAdapterHandler.
     */
    std::vector<std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterHandlerInterface>> m_externalMediaAdapterList;
    /**
     * A map of playerIds to their corresponding @c ExternalMediaAdapterHandler handlers. A playerId in this map
     * was authorized by an AuthorizeDiscoveredPlayers directive and the ExternalMediaAdapterHandler that is responsible
     * for it.
     */
    std::unordered_map<std::string, std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterHandlerInterface>>
        m_externalMediaAdapterMap;
    /**
     * A set of @c LocalMediaSource sources corresponding to registered adapters.
     */
    std::set<aace::alexa::LocalMediaSource::Source> m_registeredLocalMediaSources;
    /**
     * A map of discovered players not yet acknowledged by an AuthorizeDiscoveredPlayers directive. Key is the player's 
     * localPlayerId and value is its @c DiscoveredPlayerInfo discovery metadata. Access is serialized by 
     * @c m_playersMutex.
     */
    DiscoveredPlayerMap m_pendingDiscoveredPlayerMap;
    /**
     * A map of players reported and acknowledged by an AuthorizeDiscoveredPlayers directive. Key is the player's 
     * localPlayerId and value is @c PlayerInfo metadata including its authorization status. Access is serialized by 
     * @c m_playersMutex.
     */
    std::unordered_map<std::string, PlayerInfo> m_authorizationStateMap;
    /**
     * The current state of connection to an Alexa endpoint. Access is serialized by @c m_connectionMutex
     */
    Status m_connectionStatus = Status::DISCONNECTED;
    /**
     * Serializes access to @c m_pendingDiscoveredPlayerMap and @c m_authorizationStateMap
     */
    std::mutex m_playersMutex;
    /**
     * Serializes access to @c m_connectionStatus
     */
    std::mutex m_connectionMutex;
    /**
     * Serializes generic access 
     */
    std::mutex m_mutex;
    /**
     * Serializes generic condition 
     */
    std::condition_variable m_attemptedSetFocusPlayerInFocusCondition;

    /**
     * Condition variable for thread to wait for 
     */
    std::shared_ptr<aace::alexa::GlobalPreset> m_globalPresetHandler;

    /**
     * Only blocking operations, such as those requiring @c m_playersMutex, performed from asynchronous API calls not 
     * meant to block must be queued in this @c Executor. For instance, async handling of @c 
     * DiscoveredPlayerSenderInterface or @c ConnectionStatusObserverInterface calls may be done on this thread, but 
     * synchronous @c ExternalMediaAdapterHandlerInterface calls should not, as the capability agent already handles
     * synchronization there.
     * 
     * @c note: failing to submit to this executor async calls from the platform interface implementation that require 
     * @c m_playersMutex may result in deadlock.
     */
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    /// Mutex to serialize access to the observers.
    std::mutex m_observersMutex;
};

//
// AudioPlayerObserverDelegate
//

class AudioPlayerObserverDelegate : public alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface {
public:
    AudioPlayerObserverDelegate() = default;
    void onPlayerActivityChanged(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context) override;
    void setDelegate(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> delegate);

private:
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> m_delegate;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_EXTERNAL_MEDIA_PLAYER_ENGINE_IMPL_H
