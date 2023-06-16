/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <sstream>
#include <string>

#include <AVSCommon/AVS/EventBuilder.h>
#include <AVSCommon/Utils/UUIDGeneration/UUIDGeneration.h>

#include <AACE/Engine/Alexa/AudioPlayerEngineImpl.h>
#include <AACE/Engine/Alexa/ExternalMediaAdapterEngineImpl.h>
#include <AACE/Engine/Alexa/ExternalMediaAdapterRegistrationInterface.h>
#include <AACE/Engine/Alexa/ExternalMediaPlayerEngineImpl.h>
#include <AACE/Engine/Alexa/LocalMediaSourceEngineImpl.h>
#include <AACE/Engine/Metrics/CounterDataPointBuilder.h>
#include <AACE/Engine/Metrics/DurationDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>
#include <AACE/Engine/Metrics/StringDataPointBuilder.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Core/EngineService.h>
#include <AACE/Engine/Utils/Agent/AgentId.h>
#include <AACE/Engine/Utils/JSON/JSON.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.ExternalMediaPlayerEngineImpl");

static const std::string GLOBAL_PRESET_KEY = "preset";

/// Prefix for metrics emitted from ExternalMediaPlayer components.
static const std::string METRIC_PREFIX = "EMP-";

/// Source name for ExternalMediaPlayer request metrics
static const std::string METRIC_SOURCE_EMP_REQUEST = METRIC_PREFIX + "ExternalMediaPlayer";

/// Metric key for count of EMP requests
static const std::string METRIC_EMP_REQUEST_COUNT = "EMPRequestCount";

/// Metric key for EMP request type dimension.
static const std::string METRIC_EMP_REQUEST_TYPE_KEY = "RequestType";

/// Metric key for EMP player ID dimension.
static const std::string METRIC_EMP_PLAYER_ID_KEY = "PlayerID";

/// Metric key for count of authorization denials
static const std::string METRIC_EMP_AUTHORIZATION_DENIED_COUNT = "EMPAuthorizationDeniedCount";

/// Metric key for connection type dimension
static const std::string METRIC_CONNECTION_TYPE_KEY = "ConnectionType";

/// Metric dimension request type Login
static const std::string METRIC_REQUEST_TYPE_LOGIN = "Login";

/// Metric dimension request type Logout
static const std::string METRIC_REQUEST_TYPE_LOGOUT = "Logout";

/// Metric dimension request type for playing named content
static const std::string METRIC_REQUEST_TYPE_PLAY_CONTENT = "PlayContent";

/// Metric dimension request type for radio station tuning by frequency
static const std::string METRIC_REQUEST_TYPE_PLAY_STATION_FREQUENCY = "PlayStationFrequency";

/// Metric dimension request type for DAB channel
static const std::string METRIC_REQUEST_TYPE_PLAY_CHANNEL = "PlayChannel";

/// Metric dimension request type for play preset
static const std::string METRIC_REQUEST_TYPE_PLAY_PRESET = "PlayPreset";

/// Metric dimension request type for seek
static const std::string METRIC_REQUEST_TYPE_SEEK = "Seek";

/// Metric dimension request type for relative seek
static const std::string METRIC_REQUEST_TYPE_ADJUST_SEEK = "AdjustSeek";

/// Metric dimension connection type hybrid
static const std::string METRIC_CONNECTION_TYPE_HYBRID = "Hybrid";

/// Metric dimension connection type cloud-only
static const std::string METRIC_CONNECTION_TYPE_CLOUD = "Cloud";

/// Metric dimension connection type offline-only
static const std::string METRIC_CONNECTION_TYPE_OFFLINE = "Offline";

/// Timeout for setting focus operation.
static const std::chrono::seconds SET_FOCUS_TIMEOUT{5};

using namespace aace::engine::metrics;
using namespace aace::engine::utils::agent;

/**
 * Records a metric with the specified data points.
 * Uses the default context for ExternalMediaPlayer.
 */
static void submitMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    aace::engine::utils::agent::AgentIdType agentId,
    const std::string& source,
    const std::vector<DataPoint>& dataPoints) {
    auto metricBuilder = MetricEventBuilder()
                             .withSourceName(source)
                             .withAgentId(agentId)
                             .withIdentityType(IdentityType::NORMAL)
                             .withPriority(Priority::NORMAL)
                             .withBufferType(BufferType::NO_BUFFER);
    metricBuilder.addDataPoints(dataPoints);
    try {
        recordMetric(recorder, metricBuilder.build());
    } catch (std::invalid_argument& ex) {
        AACE_ERROR(LX(TAG).m("Failed to record metric").d("reason", ex.what()));
    }
}

/// Record a request count metric of the specified type
static void submitRequestCountMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    aace::engine::utils::agent::AgentIdType agentId,
    const std::string& requestType,
    const std::string& playerId) {
    std::vector<DataPoint> dps = {
        CounterDataPointBuilder{}.withName(METRIC_EMP_REQUEST_COUNT).increment(1).build(),
        StringDataPointBuilder{}.withName(METRIC_EMP_REQUEST_TYPE_KEY).withValue(requestType).build(),
        StringDataPointBuilder{}
            .withName(METRIC_EMP_PLAYER_ID_KEY)
            .withValue(!playerId.empty() ? playerId : "DEFAULT")
            .build()};
    submitMetric(recorder, agentId, METRIC_SOURCE_EMP_REQUEST, dps);
}

/// Record an authorization denied count metric
static void submitAuthDeniedCountMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    aace::engine::utils::agent::AgentIdType agentId,
    const std::string& playerId) {
    std::vector<DataPoint> dps = {
        CounterDataPointBuilder{}.withName(METRIC_EMP_AUTHORIZATION_DENIED_COUNT).increment(1).build(),
        StringDataPointBuilder{}.withName(METRIC_EMP_PLAYER_ID_KEY).withValue(playerId).build()};
    submitMetric(recorder, agentId, METRIC_SOURCE_EMP_REQUEST, dps);
}

ExternalMediaPlayerEngineImpl::ExternalMediaPlayerEngineImpl(const std::string& agent) :
        ExternalMediaAdapterHandlerInterface(agent), m_agent(agent) {
}

bool ExternalMediaPlayerEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedMessageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
    std::shared_ptr<aace::engine::alexa::AudioPlayerObserverDelegate> audioPlayerObserverDelegate,
    std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder,
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface> externalMediaAdapterRegistration) {
    try {
        AACE_VERBOSE(LX(TAG));

        ThrowIfNull(capabilitiesRegistrar, "invalidCapabilitiesRegistrar");
        ThrowIfNull(messageSender, "invalidMessageSender");
        ThrowIfNull(audioPlayerObserverDelegate, "invalidAudioPlayerObserverDelegate");
        ThrowIfNull(metricRecorder, "invalidMetricRecorderServiceInterface");

        m_externalMediaPlayerCapabilityAgent = aace::engine::alexa::ExternalMediaPlayer::create(
            m_agent,
            speakerManager,
            messageSender,
            certifiedMessageSender,
            focusManager,
            contextManager,
            exceptionSender,
            playbackRouter,
            externalMediaAdapterRegistration);
        ThrowIfNull(m_externalMediaPlayerCapabilityAgent, "couldNotCreateCapabilityAgent");

        // delegate AudioPlayerObserverInterface
        audioPlayerObserverDelegate->setDelegate(m_externalMediaPlayerCapabilityAgent);

        // register capability with the default endpoint
        capabilitiesRegistrar->withCapability(
            m_externalMediaPlayerCapabilityAgent, m_externalMediaPlayerCapabilityAgent);

        // add ourself as an adapter handler in the external media player capability agent
        m_externalMediaPlayerCapabilityAgent->addAdapterHandler(
            std::dynamic_pointer_cast<aace::engine::alexa::ExternalMediaAdapterHandlerInterface>(shared_from_this()));

        m_messageSender = messageSender;
        m_speakerManager = speakerManager;
        m_metricRecorder = metricRecorder;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<ExternalMediaPlayerEngineImpl> ExternalMediaPlayerEngineImpl::create(
    const std::string& agent,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedMessageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
    std::shared_ptr<aace::engine::alexa::AudioPlayerObserverDelegate> audioPlayerObserverDelegate,
    std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder,
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface> externalMediaAdapterRegistration,
    bool duckingEnabled) {
    std::shared_ptr<ExternalMediaPlayerEngineImpl> externalMediaPlayerEngineImpl = nullptr;

    try {
        // validate agent
        ThrowIf(agent.empty(), "invalidAgent");

        // create the external media player impl
        externalMediaPlayerEngineImpl =
            std::shared_ptr<ExternalMediaPlayerEngineImpl>(new ExternalMediaPlayerEngineImpl(agent));
        ThrowIfNot(
            externalMediaPlayerEngineImpl->initialize(
                capabilitiesRegistrar,
                speakerManager,
                messageSender,
                certifiedMessageSender,
                focusManager,
                contextManager,
                exceptionSender,
                playbackRouter,
                audioPlayerObserverDelegate,
                metricRecorder,
                externalMediaAdapterRegistration),
            "initializeExternalMediaPlayerEngineImplFailed");

        return externalMediaPlayerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        if (externalMediaPlayerEngineImpl != nullptr) {
            externalMediaPlayerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

bool ExternalMediaPlayerEngineImpl::registerPlatformMediaAdapter(
    std::shared_ptr<aace::alexa::ExternalMediaAdapter> platformMediaAdapter) {
    try {
        ThrowIfNull(platformMediaAdapter, "invalidExternalMediaAdapter");

        // create the ExternalMediaAdapterEngineImpl instance
        auto externalMediaAdapterEngineImpl = ExternalMediaAdapterEngineImpl::create(
            platformMediaAdapter,
            shared_from_this(),
            shared_from_this(),
            m_messageSender.lock(),
            m_speakerManager.lock(),
            m_metricRecorder.lock());
        ThrowIfNull(externalMediaAdapterEngineImpl, "invalidExternalMediaAdapterEngineImpl");

        // add the ExternalMediaAdapterEngineImpl to the ExternalMediaAdapterEngineImpl list
        std::lock_guard<std::mutex> lock(m_playersMutex);
        m_externalMediaAdapterList.push_back(externalMediaAdapterEngineImpl);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::registerPlatformMediaAdapter(
    std::shared_ptr<aace::alexa::LocalMediaSource> platformMediaAdapter) {
    try {
        AACE_DEBUG(LX(TAG));

        ThrowIfNull(platformMediaAdapter, "invalidExternalMediaAdapter");

        // get the source type from
        auto source = platformMediaAdapter->getSource();

        // check if the source is already registered with the media player
        ThrowIf(
            m_registeredLocalMediaSources.find(source) != m_registeredLocalMediaSources.end(),
            "localMediaSourceAlreadyRegistered");

        ThrowIf(
            m_defaultExternalMediaAdapter != nullptr && source == aace::alexa::LocalMediaSource::Source::DEFAULT,
            "DEFAULT localMediaSourceAlreadyRegistered");

        // get the local player id for the local media source type
        auto localPlayerId = getLocalPlayerIdForSource(source);

        // create the ExternalMediaAdapterEngineImpl instance
        auto localMediaSourceEngineImpl = LocalMediaSourceEngineImpl::create(
            platformMediaAdapter,
            localPlayerId,
            shared_from_this(),
            shared_from_this(),
            m_messageSender.lock(),
            m_speakerManager.lock(),
            m_metricRecorder.lock());
        ThrowIfNull(localMediaSourceEngineImpl, "invalidExternalMediaAdapterEngineImpl");

        std::lock_guard<std::mutex> lock(m_playersMutex);
        if (source == aace::alexa::LocalMediaSource::Source::DEFAULT) {
            AACE_VERBOSE(LX(TAG).d("platformMediaAdapter", "DEFAULT"));
            m_defaultExternalMediaAdapter = localMediaSourceEngineImpl;
        } else {
            // add the ExternalMediaAdapterEngineImpl to the ExternalMediaAdapterEngineImpl list
            m_externalMediaAdapterList.push_back(localMediaSourceEngineImpl);
        }
        m_registeredLocalMediaSources.emplace(source);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::registerPlatformGlobalPresetHandler(
    std::shared_ptr<aace::alexa::GlobalPreset> globalPresetHandler) {
    try {
        ThrowIfNull(globalPresetHandler, "invalidGlobalPresetHandler");

        m_globalPresetHandler = globalPresetHandler;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterHandlerInterface> ExternalMediaPlayerEngineImpl::getAdapter(
    const std::string& playerId) {
    AACE_VERBOSE(LX(TAG).d("playerId", playerId));
    if (playerId.empty()) {
        return m_defaultExternalMediaAdapter;
    } else {
        auto it = m_externalMediaAdapterMap.find(playerId);
        return it != m_externalMediaAdapterMap.end() ? it->second : nullptr;
    }
}

std::shared_ptr<aace::engine::alexa::ExternalMediaPlayer> ExternalMediaPlayerEngineImpl::
    getExternalMediaPlayerCapabilityAgent() {
    return m_externalMediaPlayerCapabilityAgent;
}

// aace::engine::alexa::ExternalMediaAdapterHandlerInterface

std::vector<aace::engine::alexa::PlayerInfo> ExternalMediaPlayerEngineImpl::authorizeDiscoveredPlayers(
    const std::vector<aace::engine::alexa::PlayerInfo>& authorizedPlayerList) {
    AACE_VERBOSE(LX(TAG));
    try {
        std::vector<aace::engine::alexa::PlayerInfo> acknowledgedPlayerList;
        std::unique_lock<std::mutex> lock(m_playersMutex);
        for (unsigned int j = 0; j < authorizedPlayerList.size(); j++) {
            try {
                aace::engine::alexa::PlayerInfo acknowledgedPlayerInfo = authorizedPlayerList[j];
                AACE_INFO(LX(TAG)
                              .d("localPlayerId", acknowledgedPlayerInfo.localPlayerId)
                              .d("playerId", acknowledgedPlayerInfo.playerId)
                              .d("authorized", acknowledgedPlayerInfo.authorized));

                bool wasAuthorized = acknowledgedPlayerInfo.authorized;
                if (m_pendingDiscoveredPlayerMap.find(acknowledgedPlayerInfo.localPlayerId) !=
                    m_pendingDiscoveredPlayerMap.end()) {
                    // Player has been acknowledged by cloud or local skill. If authorized, we don't need to include
                    // the player in further ReportDiscoveredPlayers events (except those triggered by a new engine
                    // connection type, which will include all known players). Since there may be both cloud and
                    // local skills, if one connection type did not authorize the player, keep it in the pending map
                    // in case the other connection will authorize it (e.g. some player is available online but not
                    // offline). Also don't include the de-authorized player in m_authorizationStateMap so it does
                    // not get included in the AuthorizationComplete event "deauthorized" array; This could be
                    // problematic, for example, if cloud authorizes a player but an AuthorizationComplete with the
                    // player in deauthorized is sent to cloud due to the player being deauthorized by the local skill.
                    if (wasAuthorized) {
                        AACE_VERBOSE(LX(TAG).d(
                            "removingPlayerFromPendingAuthorizationList", acknowledgedPlayerInfo.localPlayerId));
                        m_pendingDiscoveredPlayerMap.erase(acknowledgedPlayerInfo.localPlayerId);
                        acknowledgedPlayerList.push_back(acknowledgedPlayerInfo);
                        m_authorizationStateMap[acknowledgedPlayerInfo.localPlayerId] = acknowledgedPlayerInfo;
                    } else {
                        AACE_INFO(
                            LX(TAG).d("keepingPlayerInPendingAuthorizationList", acknowledgedPlayerInfo.localPlayerId));
                    }
                }

                if (wasAuthorized == false) {
                    AACE_WARN(LX(TAG)
                                  .m("Player authorization was denied")
                                  .d("localPlayerId", acknowledgedPlayerInfo.localPlayerId));
                    submitAuthDeniedCountMetric(
                        m_metricRecorder.lock(), AGENT_ID_NONE, acknowledgedPlayerInfo.localPlayerId);
                }

            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG).d("reason", ex.what()));
            }
        }

        // call the platform media adapter authorization method
        if (acknowledgedPlayerList.empty() == false) {
            for (auto& nextAdapter : m_externalMediaAdapterList) {
                auto acknowledgedPlayers = nextAdapter->authorizeDiscoveredPlayers(acknowledgedPlayerList);

                // add the authorized players to the media adapter player id map
                for (auto& nextPlayerInfo : acknowledgedPlayers) {
                    if (nextPlayerInfo.authorized) {
                        AACE_VERBOSE(LX(TAG)
                                         .m("adding adapter for authorized player ID to adapter map")
                                         .d("playerId", nextPlayerInfo.playerId));
                        m_externalMediaAdapterMap[nextPlayerInfo.playerId] = nextAdapter;
                    }
                }
            }
        }
        // send the authorization complete event
        auto event = createAuthorizationCompleteEventLocked();
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(
            alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_ALL, event);
        lock.unlock();

        AACE_VERBOSE(LX(TAG).m("sendingAuthorizationCompleteEvent"));
        auto messageSender = m_messageSender.lock();
        ThrowIfNull(messageSender, "invalidMessageSender");
        messageSender->sendMessage(request);
        return acknowledgedPlayerList;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::vector<aace::engine::alexa::PlayerInfo>();
    }
}

bool ExternalMediaPlayerEngineImpl::login(
    const std::string& playerId,
    const std::string& accessToken,
    const std::string& username,
    bool forceLogin,
    std::chrono::milliseconds tokenRefreshIntervalInMilliseconds) {
    try {
        AACE_VERBOSE(LX(TAG).d("playerId", playerId));
        submitRequestCountMetric(
            m_metricRecorder.lock(), aace::engine::utils::agent::AGENT_ID_NONE, METRIC_REQUEST_TYPE_LOGIN, playerId);

        auto adapter = getAdapter(playerId);
        ThrowIfNull(adapter, "invalidMediaPlayerAdapter");

        // call the adapter method
        ThrowIfNot(
            adapter->login(playerId, accessToken, username, forceLogin, tokenRefreshIntervalInMilliseconds),
            "adapterLoginFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::logout(const std::string& playerId) {
    try {
        AACE_VERBOSE(LX(TAG).d("playerId", playerId));
        submitRequestCountMetric(
            m_metricRecorder.lock(), aace::engine::utils::agent::AGENT_ID_NONE, METRIC_REQUEST_TYPE_LOGOUT, playerId);

        auto adapter = getAdapter(playerId);
        ThrowIfNull(adapter, "invalidMediaPlayerAdapter");

        // call the adapter method
        ThrowIfNot(adapter->logout(playerId), "adapterLogoutFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::play(
    const std::string& playerId,
    const std::string& playbackContextToken,
    const int64_t index,
    const std::chrono::milliseconds offsetInMilliseconds,
    const std::string& skillToken,
    const std::string& playbackSessionId,
    const std::string& navigation,
    const bool preload,
    const alexaClientSDK::avsCommon::avs::PlayRequestor& playRequestor) {
    try {
        AACE_INFO(LX(TAG).d("playerId", playerId).sensitive("playbackContextToken", playbackContextToken));
        // backward compatibility for deprecated GlobalPreset interface
        // empty playerId is a generic preset
        if (playerId.empty()) {
            std::string token = playbackContextToken;
            size_t gpindex = token.find(GLOBAL_PRESET_KEY);
            if (gpindex != std::string::npos) {
                std::string presetString =
                    token.substr(gpindex + GLOBAL_PRESET_KEY.length() + 1);  //  from "preset:" to end
                int preset = std::stoi(presetString);
                if (m_globalPresetHandler) {
                    // send global preset to platform
                    submitRequestCountMetric(
                        m_metricRecorder.lock(),
                        aace::engine::utils::agent::AGENT_ID_NONE,
                        METRIC_REQUEST_TYPE_PLAY_PRESET,
                        playerId);
                    m_globalPresetHandler->setGlobalPreset(preset);
                    return true;
                }
            }
        }
        std::string contentType;
        if (playbackContextToken.find(":frequency") != std::string::npos) {
            contentType = METRIC_REQUEST_TYPE_PLAY_STATION_FREQUENCY;
        } else if (
            playbackContextToken.find(":channel") != std::string::npos ||
            playbackContextToken.find(":dabchannel") != std::string::npos) {
            contentType = METRIC_REQUEST_TYPE_PLAY_CHANNEL;
        } else if (playbackContextToken.find(":preset")) {
            contentType = METRIC_REQUEST_TYPE_PLAY_PRESET;
        } else {
            contentType = METRIC_REQUEST_TYPE_PLAY_CONTENT;
        }
        submitRequestCountMetric(
            m_metricRecorder.lock(), aace::engine::utils::agent::AGENT_ID_NONE, contentType, playerId);

        // get the platform adapter
        auto adapter = getAdapter(playerId);
        ThrowIfNull(adapter, "invalidMediaPlayerAdapter");

        // call the adapter method
        ThrowIfNot(
            adapter->play(
                playerId,
                playbackContextToken,
                index,
                offsetInMilliseconds,
                skillToken,
                playbackSessionId,
                navigation,
                preload,
                playRequestor),
            "adapterPlayFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::playControl(const std::string& playerId, aace::engine::alexa::RequestType request) {
    try {
        AACE_VERBOSE(LX(TAG).d("playerId", playerId).d("request", request));
        std::stringstream ss;
        ss << request;
        submitRequestCountMetric(
            m_metricRecorder.lock(), aace::engine::utils::agent::AGENT_ID_NONE, ss.str(), playerId);

        // get the platform adapter
        auto adapter = getAdapter(playerId);
        ThrowIfNull(adapter, "invalidMediaPlayerAdapter");
        // call the adapter method
        ThrowIfNot(adapter->playControl(playerId, request), "adapterPlayControlFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::seek(const std::string& playerId, std::chrono::milliseconds positionMilliseconds) {
    try {
        AACE_VERBOSE(LX(TAG).d("playerId", playerId).d("positionMilliseconds", positionMilliseconds.count()));
        submitRequestCountMetric(
            m_metricRecorder.lock(), aace::engine::utils::agent::AGENT_ID_NONE, METRIC_REQUEST_TYPE_SEEK, playerId);

        // get the platform adapter
        auto adapter = getAdapter(playerId);
        ThrowIfNull(adapter, "invalidMediaPlayerAdapter");

        // call the adapter method
        ThrowIfNot(adapter->seek(playerId, positionMilliseconds), "adapterSeekFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::adjustSeek(
    const std::string& playerId,
    std::chrono::milliseconds deltaPositionMilliseconds) {
    try {
        AACE_VERBOSE(LX(TAG).d("playerId", playerId).d("deltaPositionMilliseconds", deltaPositionMilliseconds.count()));
        submitRequestCountMetric(
            m_metricRecorder.lock(),
            aace::engine::utils::agent::AGENT_ID_NONE,
            METRIC_REQUEST_TYPE_ADJUST_SEEK,
            playerId);

        // get the platform adapter
        auto adapter = getAdapter(playerId);
        ThrowIfNull(adapter, "invalidMediaPlayerAdapter");

        // call the adapter method
        ThrowIfNot(adapter->adjustSeek(playerId, deltaPositionMilliseconds), "adapterAdjustSeekFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::vector<aace::engine::alexa::AdapterState> ExternalMediaPlayerEngineImpl::getAdapterStates(bool all) {
    try {
        AACE_VERBOSE(LX(TAG));

        std::lock_guard<std::mutex> lock(m_playersMutex);
        std::vector<aace::engine::alexa::AdapterState> adapterStateList;

        // iterate through the media adapter list and add all of the adapter states
        // for the players that the adapter handles...
        for (auto next : m_externalMediaAdapterList) {
            auto adapterStates = next->getAdapterStates(all);
            adapterStateList.insert(adapterStateList.end(), adapterStates.begin(), adapterStates.end());
        }
        if (m_defaultExternalMediaAdapter != nullptr) {
            auto adapterStates = m_defaultExternalMediaAdapter->getAdapterStates(all);
            adapterStateList.insert(adapterStateList.end(), adapterStates.begin(), adapterStates.end());
        }

        return adapterStateList;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::vector<aace::engine::alexa::AdapterState>();
    }
}

std::chrono::milliseconds ExternalMediaPlayerEngineImpl::getOffset(const std::string& playerId) {
    try {
        AACE_VERBOSE(LX(TAG).d("playerId", playerId));

        // get the platform adapter
        auto adapter = getAdapter(playerId);
        ThrowIfNull(adapter, "invalidMediaPlayerAdapter");

        // call the adapter method
        return adapter->getOffset(playerId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("playerId", playerId));
        return std::chrono::milliseconds::zero();
    }
}

std::string ExternalMediaPlayerEngineImpl::createReportDiscoveredPlayersEventLocked(
    const DiscoveredPlayerMap& discoveredPlayers) {
    try {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document(rapidjson::kObjectType);

        // create payload data
        auto payload = document.GetObject();

        // add agent
        payload.AddMember(
            "agent",
            rapidjson::Value().SetString(m_agent.c_str(), m_agent.length(), document.GetAllocator()),
            document.GetAllocator());

        // add players
        rapidjson::Value playerList(rapidjson::kArrayType);

        for (const auto& next : discoveredPlayers) {
            auto info = next.second;

            rapidjson::Value player(rapidjson::kObjectType);

            player.AddMember(
                "localPlayerId",
                rapidjson::Value().SetString(
                    info.localPlayerId.c_str(), info.localPlayerId.length(), document.GetAllocator()),
                document.GetAllocator());
            player.AddMember(
                "spiVersion",
                rapidjson::Value().SetString(
                    info.spiVersion.c_str(), info.spiVersion.length(), document.GetAllocator()),
                document.GetAllocator());
            player.AddMember(
                "validationMethod",
                rapidjson::Value().SetString(
                    info.validationMethod.c_str(), info.validationMethod.length(), document.GetAllocator()),
                document.GetAllocator());

            // create the validation data
            rapidjson::Value validationData(rapidjson::kArrayType);

            for (auto& nextValidationData : info.validationData) {
                validationData.PushBack(
                    rapidjson::Value().SetString(
                        nextValidationData.c_str(), nextValidationData.length(), document.GetAllocator()),
                    document.GetAllocator());
            }

            player.AddMember("validationData", validationData, document.GetAllocator());

            // add the player to the playerList array
            playerList.PushBack(rapidjson::Value().CopyFrom(player, document.GetAllocator()), document.GetAllocator());
        }

        // add the player list to the payload
        payload.AddMember("players", playerList, document.GetAllocator());

        return alexaClientSDK::avsCommon::avs::buildJsonEventString(
                   "ExternalMediaPlayer", "ReportDiscoveredPlayers", "", aace::engine::utils::json::toString(document))
            .second;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

std::string ExternalMediaPlayerEngineImpl::createAuthorizationCompleteEventLocked() {
    try {
        AACE_VERBOSE(LX(TAG));

        rapidjson::Document document(rapidjson::kObjectType);

        // create payload data
        auto payload = document.GetObject();

        rapidjson::Value authorizedList(rapidjson::kArrayType);
        rapidjson::Value deauthorizedList(rapidjson::kArrayType);

        // add authorized and deauthorized lists
        for (const auto& next : m_authorizationStateMap) {
            auto info = next.second;

            rapidjson::Value player(rapidjson::kObjectType);

            if (info.authorized) {
                player.AddMember(
                    "playerId",
                    rapidjson::Value().SetString(
                        info.playerId.c_str(), info.playerId.length(), document.GetAllocator()),
                    document.GetAllocator());
                player.AddMember(
                    "skillToken",
                    rapidjson::Value().SetString(
                        info.skillToken.c_str(), info.skillToken.length(), document.GetAllocator()),
                    document.GetAllocator());
                authorizedList.PushBack(
                    rapidjson::Value().CopyFrom(player, document.GetAllocator()), document.GetAllocator());
            } else {
                player.AddMember(
                    "localPlayerId",
                    rapidjson::Value().SetString(
                        info.localPlayerId.c_str(), info.localPlayerId.length(), document.GetAllocator()),
                    document.GetAllocator());
                deauthorizedList.PushBack(
                    rapidjson::Value().CopyFrom(player, document.GetAllocator()), document.GetAllocator());
            }
        }

        payload.AddMember("authorized", authorizedList, document.GetAllocator());
        payload.AddMember("deauthorized", deauthorizedList, document.GetAllocator());

        return alexaClientSDK::avsCommon::avs::buildJsonEventString(
                   "ExternalMediaPlayer", "AuthorizationComplete", "", aace::engine::utils::json::toString(document))
            .second;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

// DiscoveredPlayerSenderInterface
void ExternalMediaPlayerEngineImpl::reportDiscoveredPlayers(
    const std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>& discoveredPlayers) {
    AACE_INFO(LX(TAG));
    m_executor.submit([this, discoveredPlayers]() {
        try {
            AACE_VERBOSE(LX(TAG, "reportDiscoveredPlayersExec"));

            std::lock_guard<std::mutex> lock(m_playersMutex);

            for (auto& next : discoveredPlayers) {
                if (next.localPlayerId.empty()) {
                    AACE_VERBOSE(LX(TAG, "Default player without a name does not require discovery"));
                    continue;
                }
                if (m_pendingDiscoveredPlayerMap.find(next.localPlayerId) == m_pendingDiscoveredPlayerMap.end()) {
                    AACE_INFO(LX(TAG, "reportDiscoveredPlayersExec")
                                  .m("add discovered player")
                                  .d("localPlayerId", next.localPlayerId));
                    m_pendingDiscoveredPlayerMap[next.localPlayerId] = next;
                    m_allDiscoveredPlayersMap[next.localPlayerId] = next;
                } else {
                    AACE_WARN(
                        LX(TAG).d("reason", "discoveredPlayerAlreadyPending").d("localPlayerId", next.localPlayerId));
                }
            }

            // send the pending discovered players if possible
            sendDiscoveredPlayersIfReadyLocked(m_pendingDiscoveredPlayerMap);

        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
        }
    });
}

void ExternalMediaPlayerEngineImpl::removeDiscoveredPlayer(const std::string& localPlayerId) {
    AACE_VERBOSE(LX(TAG).d("removingPlayerId", localPlayerId));
    // if the player is removed while in focus, drop focus
    if (localPlayerId.compare(m_externalMediaPlayerCapabilityAgent->getPlayerInFocus()) == 0) {
        AACE_VERBOSE(LX(TAG).m("removing focus in capability agent").d("local)layerId", localPlayerId));
        m_externalMediaPlayerCapabilityAgent->setPlayerInFocus(m_authorizationStateMap[localPlayerId].playerId, false);
        AACE_VERBOSE(
            LX(TAG, "setPlayerInFocus to false for player due to discovery removal").d("localPlayerId", localPlayerId));
    }

    m_executor.submit([this, localPlayerId]() {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        AACE_DEBUG(LX(TAG).d("Removing localPlayerId", localPlayerId));
        m_authorizationStateMap.erase(localPlayerId);
        m_pendingDiscoveredPlayerMap.erase(localPlayerId);
        m_allDiscoveredPlayersMap.erase(localPlayerId);
    });
}

void ExternalMediaPlayerEngineImpl::sendDiscoveredPlayersIfReadyLocked(const DiscoveredPlayerMap& discoveredPlayers) {
    try {
        AACE_VERBOSE(LX(TAG));

        std::unique_lock<std::mutex> lock(m_connectionMutex);
        auto connectionStatus = m_aggregateConnectionStatus;
        lock.unlock();
        if (connectionStatus == Status::CONNECTED && discoveredPlayers.empty() == false) {
            auto event = createReportDiscoveredPlayersEventLocked(discoveredPlayers);
            auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(
                alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_ALL, event);

            AACE_INFO(LX(TAG).m("sendingReportDiscoveredPlayersEvent"));

            auto m_messageSender_lock = m_messageSender.lock();
            ThrowIfNull(m_messageSender_lock, "invalidMessageSender");
            m_messageSender_lock->sendMessage(request);

            // Note: we wait to clear the pending discovery map until the players
            // are confirmed by an AuthorizeDiscoveredPlayers directive
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string ExternalMediaPlayerEngineImpl::getLocalPlayerIdForSource(aace::alexa::LocalMediaSource::Source source) {
    try {
        switch (source) {
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
                return "com.amazon.alexa.auto.players.DAB_RADIO";

            case aace::alexa::LocalMediaSource::Source::DEFAULT:
                return "";

            default:
                throw("invalidLocalMediaSource");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("source", source));
        return "";
    }
}

// FocusHandlerInterface
void ExternalMediaPlayerEngineImpl::setFocus(const std::string& playerId, bool focusAcquire) {
    AACE_INFO(LX(TAG).d("playerId", playerId).d("focusAcquire", focusAcquire));
    // TBD implement better association on localPlayerId and playerId
    for (auto& nextAdapter : m_authorizationStateMap) {
        if (nextAdapter.second.playerId.compare(playerId) == 0) {
            std::string localPlayerId = nextAdapter.second.localPlayerId;
            if (m_authorizationStateMap.find(localPlayerId) == m_authorizationStateMap.end() && focusAcquire) {
                AACE_WARN(LX(TAG, "setPlayerInFocusDelayed")
                              .d("reason", "unauthorizedPlayer")
                              .d("localPlayerId", localPlayerId));

                // check if player is pending authorization w/ 5 sec timeout
                m_executor.submit([this, localPlayerId, playerId, focusAcquire]() {
                    auto predicate = [this, localPlayerId, playerId]() {
                        if (m_authorizationStateMap.find(localPlayerId) == m_authorizationStateMap.end())
                            return false;
                        else
                            return m_authorizationStateMap[localPlayerId].authorized;
                    };
                    std::unique_lock<std::mutex> lock(m_mutex);
                    if (m_attemptedSetFocusPlayerInFocusCondition.wait_for(lock, SET_FOCUS_TIMEOUT, predicate)) {
                        // not already in focus
                        AACE_VERBOSE(LX(TAG).m("setting focus in capability agent").d("playerId", playerId));
                        m_externalMediaPlayerCapabilityAgent->setPlayerInFocus(playerId, focusAcquire);
                    } else {
                        AACE_ERROR(LX(TAG, "setPlayerInFocusFailed")
                                       .d("reason", "wait for authorize player timed out")
                                       .d("localPlayerId", localPlayerId));
                    }
                });
                // not already in focus
            } else {
                AACE_VERBOSE(LX(TAG).m("setting focus in capability agent").d("playerId", playerId));
                m_externalMediaPlayerCapabilityAgent->setPlayerInFocus(playerId, focusAcquire);
            }
        }
    }
}

void ExternalMediaPlayerEngineImpl::setDefaultPlayerFocus() {
    try {
        ThrowIfNull(m_externalMediaPlayerCapabilityAgent, "ExternalMediaPlayer is null");
        m_externalMediaPlayerCapabilityAgent->setPlayerInFocus("", true);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void ExternalMediaPlayerEngineImpl::onConnectionStatusChanged(const Status status, const ChangedReason reason) {
    // no-op
}

void ExternalMediaPlayerEngineImpl::onConnectionStatusChanged(
    const Status status,
    const std::vector<EngineConnectionStatus>& engineStatuses) {
    AACE_INFO(LX(TAG).d("status", status).d("engineCount", engineStatuses.size()));
    std::lock_guard<std::mutex> lock(m_connectionMutex);
    onConnectionStatusChangedLocked(status, engineStatuses);
}

void ExternalMediaPlayerEngineImpl::onConnectionStatusChangedLocked(
    const Status status,
    const std::vector<EngineConnectionStatus>& engineStatuses) {
    AACE_VERBOSE(LX(TAG).d("status", status).d("engineCount", engineStatuses.size()));
    bool newConnection = false;
    m_aggregateConnectionStatus = status;
    if (m_customAgentStatus == AgentAvailability::AVAILABLE) {
        AACE_INFO(LX(TAG).m("Setting aggregate status CONNECTED because custom agent is available"));
        m_aggregateConnectionStatus = Status::CONNECTED;
    }

    for (const auto& engineStatus : engineStatuses) {
        auto engineType = engineStatus.engineType;
        if (engineType == 1) {
            AACE_VERBOSE(LX(TAG).d("Setting alexa connection status", engineStatus.status));
            m_alexaConnectionStatus = engineStatus.status;
        }
        auto it = std::find(m_currentConnectedEngineTypes.begin(), m_currentConnectedEngineTypes.end(), engineType);
        auto wasPreviouslyConnected = it != m_currentConnectedEngineTypes.end();
        if (!wasPreviouslyConnected && engineStatus.status == Status::CONNECTED) {
            // The engine type is newly connected. Flag it and keep track of the connection
            AACE_VERBOSE(LX(TAG).m("adding new engine connection").d("engine type", engineType));
            m_currentConnectedEngineTypes.push_back(engineType);
            newConnection = true;
        } else if (wasPreviouslyConnected && engineStatus.status != Status::CONNECTED) {
            // The engine type is no longer connected. Remove the connection
            AACE_VERBOSE(LX(TAG).m("removing engine connection").d("engine type", engineType));
            m_currentConnectedEngineTypes.erase(it);
        }
    }

    if (newConnection) {
        // Redo authorization sequence. We either became connected to an Alexa engine when
        // previously not, or our connection switched between different Alexa engines.
        m_executor.submit([this]() {
            std::lock_guard<std::mutex> lock(m_playersMutex);
            sendDiscoveredPlayersIfReadyLocked(m_allDiscoveredPlayersMap);
        });
    }
}

void ExternalMediaPlayerEngineImpl::onAgentAvailabilityStateChanged(
    alexaClientSDK::avsCommon::avs::AgentId::IdType agentId,
    AgentAvailability status,
    const std::string& reason) {
    AACE_INFO(LX(TAG).d("agentId", agentId).d("available", status == AgentAvailability::AVAILABLE).d("reason", reason));
    // Since
    // 1. ExternalMediaPlayerEngineImpl is only added as an agent state observer
    //    in On-Device ACA policy
    // 2. In On-Device ACA policy ConnectionStatusObserverInterface does not
    //    include engine type 2 (LVC) in the calls to onConnectionStatusChanged
    // 3. In On-Device ACA policy, engine type 2 powers the custom agent
    // we can equate the the status of the custom agent to the connection status
    // of engine type 2 to re-use the handling in onConnectionStatusChanged
    if (agentId != alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_NONE &&
        agentId != alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_ALL &&
        agentId != alexaClientSDK::avsCommon::avs::AgentId::getAlexaAgentId()) {
        AACE_DEBUG(LX(TAG).m("Using status of non-Alexa agent for engine type 2 connection status"));
        std::lock_guard<std::mutex> lock(m_connectionMutex);
        m_customAgentStatus = status;
        Status engine2Status = status == AgentAvailability::AVAILABLE ? Status::CONNECTED : Status::DISCONNECTED;
        Status aggregateConnectionStatus =
            status == AgentAvailability::AVAILABLE ? Status::CONNECTED : m_alexaConnectionStatus;
        onConnectionStatusChangedLocked(
            aggregateConnectionStatus, {EngineConnectionStatus{2, ChangedReason::NONE, engine2Status}});
    }
}

void ExternalMediaPlayerEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));

    m_executor.waitForSubmittedTasks();
    m_executor.shutdown();

    m_registeredLocalMediaSources.clear();
    m_globalPresetHandler.reset();

    // shut down external media adapters
    {
        std::lock_guard<std::mutex> lock(m_playersMutex);

        for (auto& adapter : m_externalMediaAdapterList) {
            adapter->shutdown();
        }
        if (m_defaultExternalMediaAdapter != nullptr) {
            m_defaultExternalMediaAdapter->shutdown();
            m_defaultExternalMediaAdapter.reset();
        }
        // clear the external media adapter list
        m_externalMediaAdapterList.clear();

        // clear the media adapter player id map
        m_externalMediaAdapterMap.clear();
    }

    // shutdown the capability agent
    if (m_externalMediaPlayerCapabilityAgent != nullptr) {
        m_externalMediaPlayerCapabilityAgent->shutdown();
        m_externalMediaPlayerCapabilityAgent.reset();
    }

    if (!m_metricRecorder.expired()) {
        m_metricRecorder.reset();
    }
}

// RenderPlayerInfoCardObserverInterface
void ExternalMediaPlayerEngineImpl::setObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface> observer) {
    std::lock_guard<std::mutex> lock{m_observersMutex};
    m_externalMediaPlayerCapabilityAgent->setObserver(observer);
}

//
// AudioPlayerObserverDelegate
//
void AudioPlayerObserverDelegate::onPlayerActivityChanged(
    alexaClientSDK::avsCommon::avs::PlayerActivity state,
    const Context& context) {
    if (m_delegate != nullptr) {
        m_delegate->onPlayerActivityChanged(state, context);
    }
}

void AudioPlayerObserverDelegate::setDelegate(
    std::shared_ptr<alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface> delegate) {
    m_delegate = delegate;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
