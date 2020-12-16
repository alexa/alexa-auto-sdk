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

#include <AVSCommon/AVS/EventBuilder.h>
#include <AVSCommon/Utils/UUIDGeneration/UUIDGeneration.h>

#include "AACE/Engine/Alexa/AudioPlayerEngineImpl.h"
#include "AACE/Engine/Alexa/ExternalMediaAdapterEngineImpl.h"
#include "AACE/Engine/Alexa/ExternalMediaAdapterRegistrationInterface.h"
#include "AACE/Engine/Alexa/ExternalMediaPlayerEngineImpl.h"
#include "AACE/Engine/Alexa/LocalMediaSourceEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Core/EngineService.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.ExternalMediaPlayerEngineImpl");

static const std::string GLOBAL_PRESET_KEY = "preset";

/// Timeout for setting focus operation.
static const std::chrono::seconds SET_FOCUS_TIMEOUT{5};

ExternalMediaPlayerEngineImpl::ExternalMediaPlayerEngineImpl(const std::string& agent) :
        ExternalMediaAdapterHandlerInterface(agent), m_agent(agent) {
}

bool ExternalMediaPlayerEngineImpl::initialize(
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
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface> externalMediaAdapterRegistration) {
    try {
        AACE_VERBOSE(LX(TAG));

        ThrowIfNull(defaultEndpointBuilder, "invalidDefaultEndpointBuilder");
        ThrowIfNull(capabilitiesDelegate, "invalidCapabilitiesDelegate");
        ThrowIfNull(messageSender, "invalidMessageSender");

        m_externalMediaPlayerCapabilityAgent = aace::engine::alexa::ExternalMediaPlayer::create(
            m_agent,
            {},
            {},
            {},
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
        defaultEndpointBuilder->withCapability(
            m_externalMediaPlayerCapabilityAgent, m_externalMediaPlayerCapabilityAgent);

        // add ourself as an adapter handler in the external media player capability agent
        m_externalMediaPlayerCapabilityAgent->addAdapterHandler(
            std::dynamic_pointer_cast<aace::engine::alexa::ExternalMediaAdapterHandlerInterface>(shared_from_this()));

        m_messageSender = messageSender;
        m_speakerManager = speakerManager;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<ExternalMediaPlayerEngineImpl> ExternalMediaPlayerEngineImpl::create(
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
    std::shared_ptr<aace::engine::alexa::ExternalMediaAdapterRegistrationInterface> externalMediaAdapterRegistration) {
    std::shared_ptr<ExternalMediaPlayerEngineImpl> externalMediaPlayerEngineImpl = nullptr;

    try {
        // validate agent
        ThrowIf(agent.empty(), "invalidAgent");

        // create the external media player impl
        externalMediaPlayerEngineImpl =
            std::shared_ptr<ExternalMediaPlayerEngineImpl>(new ExternalMediaPlayerEngineImpl(agent));
        ThrowIfNot(
            externalMediaPlayerEngineImpl->initialize(
                defaultEndpointBuilder,
                capabilitiesDelegate,
                speakerManager,
                messageSender,
                certifiedMessageSender,
                focusManager,
                contextManager,
                exceptionSender,
                playbackRouter,
                audioPlayerObserverDelegate,
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
            m_speakerManager.lock());
        ThrowIfNull(externalMediaAdapterEngineImpl, "invalidExternalMediaAdapterEngineImpl");

        // add the ExternalMediaAdapterEngineImpl to the ExternalMediaAdapterEngineImpl list
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

        // get the local player id for the local media source type
        auto localPlayerId = getLocalPlayerIdForSource(source);
        ThrowIf(localPlayerId.empty(), "invalidLocalPlayerId");

        // create the ExternalMediaAdapterEngineImpl instance
        auto localMediaSourceEngineImpl = LocalMediaSourceEngineImpl::create(
            platformMediaAdapter,
            localPlayerId,
            shared_from_this(),
            shared_from_this(),
            m_messageSender.lock(),
            m_speakerManager.lock());
        ThrowIfNull(localMediaSourceEngineImpl, "invalidExternalMediaAdapterEngineImpl");

        // add the ExternalMediaAdapterEngineImpl to the ExternalMediaAdapterEngineImpl list
        m_externalMediaAdapterList.push_back(localMediaSourceEngineImpl);
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
    auto it = m_externalMediaAdapterMap.find(playerId);
    return it != m_externalMediaAdapterMap.end() ? it->second : nullptr;
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

                if (m_pendingDiscoveredPlayerMap.find(acknowledgedPlayerInfo.localPlayerId) !=
                    m_pendingDiscoveredPlayerMap.end()) {
                    // Player has been acknowledged by cloud or local skill. Safe to exclude from future
                    // ReportDiscoveredPlayers events
                    AACE_VERBOSE(
                        LX(TAG).d("removingPlayerFromPendingDiscoveryList", acknowledgedPlayerInfo.localPlayerId));
                    m_pendingDiscoveredPlayerMap.erase(acknowledgedPlayerInfo.localPlayerId);
                }
                // add the player info to the authorized player list
                acknowledgedPlayerList.push_back(acknowledgedPlayerInfo);

                // add the player info to the authorized player map
                m_authorizationStateMap[acknowledgedPlayerInfo.localPlayerId] = acknowledgedPlayerInfo;
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
                        m_externalMediaAdapterMap[nextPlayerInfo.playerId] = nextAdapter;
                    }
                }
            }
        }
        // send the authorization complete event
        auto event = createAuthorizationCompleteEventLocked();
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event);
        lock.unlock();
        AACE_VERBOSE(LX(TAG).m("sendingAuthorizationCompleteEvent"));
        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");
        m_messageSender_lock->sendMessage(request);
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
        AACE_VERBOSE(LX(TAG));
        if (playerId.empty()) {  // empty playerId == global preset case
            std::string token = playbackContextToken;
            size_t gpindex = token.find(GLOBAL_PRESET_KEY);
            ThrowIfNot(gpindex != std::string::npos, "invalidPayloadWithGlobalPreset");
            std::string presetString =
                token.substr(gpindex + GLOBAL_PRESET_KEY.length() + 1);  //  from "preset:" to end
            int preset = std::stoi(presetString);
            ThrowIfNull(m_globalPresetHandler, "platformGlobalPresetHandlerNull");
            // send global preset to platform
            m_globalPresetHandler->setGlobalPreset(preset);
            return true;
        } else {
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
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaPlayerEngineImpl::playControl(const std::string& playerId, aace::engine::alexa::RequestType request) {
    try {
        AACE_VERBOSE(LX(TAG).d("playerId", playerId).d("request", request));

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

        std::vector<aace::engine::alexa::AdapterState> adapterStateList;

        // iterate through the media adapter list and add all of the adapter states
        // for the players that the adapter handles...
        for (auto next : m_externalMediaAdapterList) {
            auto adapterStates = next->getAdapterStates(all);
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

        // create payload string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);

        return alexaClientSDK::avsCommon::avs::buildJsonEventString(
                   "ExternalMediaPlayer", "ReportDiscoveredPlayers", "", buffer.GetString())
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

        // create payload string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);

        return alexaClientSDK::avsCommon::avs::buildJsonEventString(
                   "ExternalMediaPlayer", "AuthorizationComplete", "", buffer.GetString())
            .second;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

// DiscoveredPlayerSenderInterface
void ExternalMediaPlayerEngineImpl::reportDiscoveredPlayers(
    const std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>& discoveredPlayers) {
    AACE_VERBOSE(LX(TAG));
    m_executor.submit([this, discoveredPlayers]() {
        try {
            AACE_VERBOSE(LX(TAG, "reportDiscoveredPlayersExec"));

            std::lock_guard<std::mutex> lock(m_playersMutex);

            for (auto& next : discoveredPlayers) {
                if (m_pendingDiscoveredPlayerMap.find(next.localPlayerId) == m_pendingDiscoveredPlayerMap.end()) {
                    AACE_VERBOSE(
                        LX(TAG).m("addingDiscoveredPlayerToPendingMap").d("localPlayerId", next.localPlayerId));
                    m_pendingDiscoveredPlayerMap[next.localPlayerId] = next;
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
        m_externalMediaPlayerCapabilityAgent->setPlayerInFocus(m_authorizationStateMap[localPlayerId].playerId, false);
        AACE_VERBOSE(
            LX(TAG, "setPlayerInFocus to false for player due to discovery removal").d("localPlayerId", localPlayerId));
    }

    m_executor.submit([this, localPlayerId]() {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        m_authorizationStateMap.erase(localPlayerId);
    });
}

void ExternalMediaPlayerEngineImpl::sendDiscoveredPlayersIfReadyLocked(const DiscoveredPlayerMap& discoveredPlayers) {
    try {
        AACE_VERBOSE(LX(TAG));

        std::unique_lock<std::mutex> lock(m_connectionMutex);
        auto connectionStatus = m_connectionStatus;
        lock.unlock();
        if (connectionStatus == Status::CONNECTED && discoveredPlayers.empty() == false) {
            auto event = createReportDiscoveredPlayersEventLocked(discoveredPlayers);
            auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event);

            AACE_VERBOSE(LX(TAG).m("sendingReportDiscoveredPlayersEvent"));

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
    AACE_VERBOSE(LX(TAG).d("playerId", playerId));
    if (playerId.empty()) {
        AACE_ERROR(LX(TAG, "setPlayerInFocusFailed").d("reason", "empty playerId"));
        return;
    }
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
                        m_externalMediaPlayerCapabilityAgent->setPlayerInFocus(playerId, focusAcquire);
                    } else {
                        AACE_ERROR(LX(TAG, "setPlayerInFocusFailed")
                                       .d("reason", "wait for authorize player timed out")
                                       .d("localPlayerId", localPlayerId));
                    }
                });
                // not already in focus
            } else
                m_externalMediaPlayerCapabilityAgent->setPlayerInFocus(playerId, focusAcquire);
        }
    }
}

// alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
void ExternalMediaPlayerEngineImpl::onConnectionStatusChanged(const Status status, const ChangedReason reason) {
    AACE_VERBOSE(LX(TAG).d("status", status));

    std::lock_guard<std::mutex> lock(m_connectionMutex);
    if (m_connectionStatus != status) {
        m_connectionStatus = status;
        m_executor.submit([this]() {
            std::lock_guard<std::mutex> lock(m_playersMutex);
            sendDiscoveredPlayersIfReadyLocked(m_pendingDiscoveredPlayerMap);
        });
    }
}

// alexaClientSDK::avsCommon::utils::RequiresShutdown
void ExternalMediaPlayerEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));

    m_registeredLocalMediaSources.clear();
    m_globalPresetHandler.reset();
    m_executor.shutdown();

    // shut down external media adapters
    for (auto& adapter : m_externalMediaAdapterList) {
        adapter->shutdown();
        adapter.reset();
    }
    // clear the external media adapter list
    m_externalMediaAdapterList.clear();

    // clear the media adapter player id map
    m_externalMediaAdapterMap.clear();

    // shutdown the capability agent
    if (m_externalMediaPlayerCapabilityAgent != nullptr) {
        m_externalMediaPlayerCapabilityAgent->shutdown();
        m_externalMediaPlayerCapabilityAgent.reset();
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
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> delegate) {
    m_delegate = delegate;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
