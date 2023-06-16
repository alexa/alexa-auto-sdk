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

#include <AVSCommon/AVS/AgentId.h>
#include <AVSCommon/AVS/EventBuilder.h>

#include <AACE/Engine/Alexa/ExternalMediaAdapterHandler.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/Agent/AgentId.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::metrics;

static const uint8_t DEFAULT_SPEAKER_VOLUME = 50;

/// Timeout for setting focus operation.
static const std::chrono::seconds SET_FOCUS_TIMEOUT{5};

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.ExternalMediaAdapterHandler");

// external media player agent constant
static const std::string EXTERNAL_MEDIA_PLAYER_AGENT = "alexaAutoSDK";

/// Prefix for metrics emitted from ExternalMediaPlayer components.
static const std::string METRIC_PREFIX = "EMP-";

/// Source name for ExternalMediaPlayer request metrics
static const std::string METRIC_SOURCE_EMP_REQUEST = METRIC_PREFIX + "ExternalMediaPlayer";

/// Metric key for EMP player ID dimension.
static const std::string METRIC_EMP_PLAYER_ID_KEY = "PlayerID";

/// Metric key for count of player events
static const std::string METRIC_EMP_PLAYER_EVENT_COUNT = "EMPPlayerEventCount";

/// Metric key for EMP player event type dimension.
static const std::string METRIC_EMP_PLAYER_EVENT_TYPE_KEY = "EventType";

/// Metric key for count of authorization denials
static const std::string METRIC_EMP_PLAYER_ERROR_COUNT = "EMPPlayerErrorCount";

/// Metric key for EMP player error type dimension.
static const std::string METRIC_EMP_PLAYER_ERROR_TYPE_KEY = "ErrorType";

/// Metric key for EMP request type dimension.
static const std::string METRIC_EMP_REQUEST_TYPE_KEY = "RequestType";

/// Metric key for latency of EMP requests
static const std::string METRIC_EMP_RESPONSE_LATENCY = "EMPResponseLatency";

/// Metric dimension request type Login
static const std::string METRIC_REQUEST_TYPE_LOGIN = "Login";

/// Metric dimension request type Logout
static const std::string METRIC_REQUEST_TYPE_LOGOUT = "Logout";

using namespace aace::engine::metrics;

void ExternalMediaAdapterHandler::submitMetric(
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

void ExternalMediaAdapterHandler::submitEventCountMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    aace::engine::utils::agent::AgentIdType agentId,
    const std::string& eventType,
    const std::string& playerId) {
    std::vector<DataPoint> dps = {
        CounterDataPointBuilder{}.withName(METRIC_EMP_PLAYER_EVENT_COUNT).increment(1).build(),
        StringDataPointBuilder{}.withName(METRIC_EMP_PLAYER_EVENT_TYPE_KEY).withValue(eventType).build(),
        StringDataPointBuilder{}
            .withName(METRIC_EMP_PLAYER_ID_KEY)
            .withValue(!playerId.empty() ? playerId : "DEFAULT")
            .build()};
    submitMetric(recorder, agentId, METRIC_SOURCE_EMP_REQUEST, dps);
}

void ExternalMediaAdapterHandler::submitErrorCountMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    aace::engine::utils::agent::AgentIdType agentId,
    const std::string& errorType,
    const std::string& playerId) {
    std::vector<DataPoint> dps = {
        CounterDataPointBuilder{}.withName(METRIC_EMP_PLAYER_ERROR_COUNT).increment(1).build(),
        StringDataPointBuilder{}.withName(METRIC_EMP_PLAYER_ERROR_TYPE_KEY).withValue(errorType).build(),
        StringDataPointBuilder{}
            .withName(METRIC_EMP_PLAYER_ID_KEY)
            .withValue(!playerId.empty() ? playerId : "DEFAULT")
            .build()};
    submitMetric(recorder, agentId, METRIC_SOURCE_EMP_REQUEST, dps);
}

void ExternalMediaAdapterHandler::submitResponseLatencyMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    aace::engine::utils::agent::AgentIdType agentId,
    const std::string& requestType,
    const std::string& playerId,
    const DataPoint& latencyDataPoint) {
    std::vector<DataPoint> dps = {
        latencyDataPoint,
        StringDataPointBuilder{}.withName(METRIC_EMP_REQUEST_TYPE_KEY).withValue(requestType).build(),
        StringDataPointBuilder{}
            .withName(METRIC_EMP_PLAYER_ID_KEY)
            .withValue(!playerId.empty() ? playerId : "DEFAULT")
            .build()};
    submitMetric(recorder, agentId, METRIC_SOURCE_EMP_REQUEST, dps);
}

ExternalMediaAdapterHandler::ExternalMediaAdapterHandler(
    std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender,
    std::shared_ptr<FocusHandlerInterface> focusHandler,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder) :
        ExternalMediaAdapterHandlerInterface::ExternalMediaAdapterHandlerInterface(TAG),
        m_focusHandler(focusHandler),
        m_messageSender(messageSender),
        m_metricRecorder(metricRecorder),
        m_discoveredPlayerSender(discoveredPlayerSender),
        m_muted(false),
        m_volume(DEFAULT_SPEAKER_VOLUME) {
}

bool ExternalMediaAdapterHandler::initializeAdapterHandler(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager) {
    try {
        ThrowIfNull(speakerManager, "invalidSpeakerManager");

        // add the speaker impl to the speaker manager
        speakerManager->addChannelVolumeInterface(shared_from_this());

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initializeAdapterHandler").d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaAdapterHandler::validatePlayer(const std::string& localPlayerId, bool checkAuthorized) {
    auto it = m_playerInfoMap.find(localPlayerId);
    return it != m_playerInfoMap.end() && (it->second.authorized || checkAuthorized == false);
}

bool ExternalMediaAdapterHandler::setFocus(const std::string& localPlayerId, bool focusAcquire) {
    AACE_INFO(LX(TAG).d("localPlayerId", localPlayerId).d("focusAcquire", focusAcquire));
    try {
        ThrowIfNot(validatePlayer(localPlayerId, false), "invalidPlayerInfo");
        auto playerInfo = m_playerInfoMap[localPlayerId];
        auto playerId = playerInfo.playerId;

        auto focusHandler_lock = m_focusHandler.lock();
        ThrowIfNull(focusHandler_lock, "invalidFocusHandler");

        if (!playerId.empty()) {
            focusHandler_lock->setFocus(playerId, focusAcquire);
            return true;
        }

        AACE_WARN(LX(TAG, "setFocusDelayed").d("reason", "unauthorizedPlayer"));

        // check if player is pending authorization w/ 5 sec timeout
        m_executor.submit([this, localPlayerId, playerId, focusAcquire, focusHandler_lock]() {
            auto predicate = [this, localPlayerId, playerId]() {
                if (m_playerInfoMap.find(localPlayerId) == m_playerInfoMap.end())
                    return false;
                else
                    return m_playerInfoMap[localPlayerId].authorized;
            };
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_attemptedSetFocusPlayerInFocusCondition.wait_for(lock, SET_FOCUS_TIMEOUT, predicate)) {
                // not already in focus
                focusHandler_lock->setFocus(playerId, focusAcquire);
            } else {
                AACE_ERROR(LX(TAG, "setPlayerInFocusFailed")
                               .d("reason", "wait for authorize player timed out")
                               .d("localPlayerId", localPlayerId));
                m_focusHandler.expired();
            }
        });

        return true;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "setFocus").d("reason", ex.what()).d("localPlayerId", localPlayerId));
        return false;
    }
}

void ExternalMediaAdapterHandler::setDefaultPlayerFocus() {
    try {
        AACE_DEBUG(LX(TAG).m("Setting focus for default player"));
        auto focusHandler_lock = m_focusHandler.lock();
        ThrowIfNull(focusHandler_lock, "invalidFocusHandler");
        m_executor.submit([focusHandler_lock]() { focusHandler_lock->setDefaultPlayerFocus(); });
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::vector<PlayerInfo> ExternalMediaAdapterHandler::authorizeDiscoveredPlayers(
    const std::vector<PlayerInfo>& authorizedPlayerList) {
    try {
        std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo> authorizedPlayerInfoList;
        std::vector<PlayerInfo> supportedPlayerList;

        for (const auto& next : authorizedPlayerList) {
            if (validatePlayer(next.localPlayerId, false)) {
                aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo info;

                info.localPlayerId = next.localPlayerId;
                info.authorized = next.authorized;
                info.defaultSkillToken = next.skillToken;
                info.playerId = next.playerId;

                authorizedPlayerInfoList.push_back(info);
                supportedPlayerList.push_back(next);

                // copy the player info into the player info map
                m_playerInfoMap[next.localPlayerId] = next;

                // add an entry to the alexa to local player id map
                m_alexaToLocalPlayerIdMap[next.playerId] = next.localPlayerId;
            }
        }

        if (authorizedPlayerInfoList.empty() == false) {
            ThrowIfNot(handleAuthorization(authorizedPlayerInfoList), "handleAuthorizeFailed");
        }

        return supportedPlayerList;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "authorizeDiscoveredPlayers").d("reason", ex.what()));
        return std::vector<PlayerInfo>();
    }
}

bool ExternalMediaAdapterHandler::login(
    const std::string& playerId,
    const std::string& accessToken,
    const std::string& userName,
    bool forceLogin,
    std::chrono::milliseconds tokenRefreshInterval) {
    try {
        m_loginDurationBuilder.startTimer();
        auto it = m_alexaToLocalPlayerIdMap.find(playerId);
        ThrowIf(it == m_alexaToLocalPlayerIdMap.end(), "invalidPlayerId");

        // call the platform media adapter
        ThrowIfNot(
            handleLogin(it->second, accessToken, userName, forceLogin, tokenRefreshInterval), "handleLoginFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "login")
                       .d("reason", ex.what())
                       .d("playerId", playerId)
                       .d("userName", userName)
                       .d("forceLogin", forceLogin));
        m_loginDurationBuilder.stopTimer();
        return false;
    }
}

bool ExternalMediaAdapterHandler::logout(const std::string& playerId) {
    try {
        m_logoutDurationBuilder.startTimer();
        auto it = m_alexaToLocalPlayerIdMap.find(playerId);
        ThrowIf(it == m_alexaToLocalPlayerIdMap.end(), "invalidPlayerId");

        // call the platform media adapter
        ThrowIfNot(handleLogout(it->second), "handleLogoutFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "logout").d("reason", ex.what()).d("playerId", playerId));
        m_logoutDurationBuilder.stopTimer();
        return false;
    }
}

static const std::unordered_map<std::string, aace::alexa::ExternalMediaAdapter::Navigation> NAVIGATION_ENUM_MAP = {
    {"DEFAULT", aace::alexa::ExternalMediaAdapter::Navigation::DEFAULT},
    {"NONE", aace::alexa::ExternalMediaAdapter::Navigation::NONE},
    {"FOREGROUND", aace::alexa::ExternalMediaAdapter::Navigation::FOREGROUND}};

static aace::alexa::ExternalMediaAdapter::Navigation getNavigationEnum(std::string name) {
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) -> unsigned char {
        return static_cast<unsigned char>(std::toupper(c));
    });

    auto it = NAVIGATION_ENUM_MAP.find(name);

    return it != NAVIGATION_ENUM_MAP.end() ? it->second : aace::alexa::ExternalMediaAdapter::Navigation::DEFAULT;
}

bool ExternalMediaAdapterHandler::play(
    const std::string& playerId,
    const std::string& playContextToken,
    int64_t index,
    std::chrono::milliseconds offset,
    const std::string& skillToken,
    const std::string& playbackSessionId,
    const std::string& navigation,
    bool preload,
    const alexaClientSDK::avsCommon::avs::PlayRequestor& playRequestor) {
    try {
        std::string localPlayerId;
        if (playerId.empty()) {
            auto it = m_playerInfoMap.find(playerId);
            ThrowIf(it == m_playerInfoMap.end(), "Default Player not found");
        } else {
            auto it = m_alexaToLocalPlayerIdMap.find(playerId);
            ThrowIf(it == m_alexaToLocalPlayerIdMap.end(), "invalidPlayerId");
            localPlayerId = it->second;
            // update the player info
            ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerInfo");
            auto playerInfo = m_playerInfoMap[localPlayerId];

            playerInfo.skillToken = skillToken;
            playerInfo.playbackSessionId = playbackSessionId;
        }

        // call the platform media adapter
        ThrowIfNot(
            handlePlay(
                localPlayerId,
                playContextToken,
                index,
                offset,
                preload,
                getNavigationEnum(navigation),
                playbackSessionId,
                skillToken),
            "handlePlayFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "play").d("reason", ex.what()).d("playerId", playerId));
        return false;
    }
}

bool ExternalMediaAdapterHandler::playControl(
    const std::string& playerId,
    aace::engine::alexa::RequestType requestType) {
    AACE_INFO(LX(TAG).d("playerId", playerId).d("requestType", requestType));
    std::string localPlayerId;
    try {
        // convert RequestType to PlayControlType
        using RequestType = aace::engine::alexa::RequestType;
        using PlayControlType = aace::alexa::ExternalMediaAdapter::PlayControlType;

        if (playerId.empty()) {
            auto it = m_playerInfoMap.find(playerId);
            ThrowIf(it == m_playerInfoMap.end(), "Default Player not found");
        } else {
            auto it = m_alexaToLocalPlayerIdMap.find(playerId);
            ThrowIf(it == m_alexaToLocalPlayerIdMap.end(), "invalidPlayerId");
            localPlayerId = it->second;
        }

        aace::engine::alexa::AdapterState state;

        ThrowIfNot(handleGetAdapterState(localPlayerId, state), "handleGetAdapterStateFailed");
        ThrowIfNot(state.sessionState.launched, "player disabled");
        PlayControlType controlType;

        switch (requestType) {
            case RequestType::PAUSE:
                controlType = PlayControlType::PAUSE;
                break;
            case RequestType::RESUME:
                controlType = PlayControlType::RESUME;
                break;
            case RequestType::STOP:
                controlType = PlayControlType::STOP;
                break;
            case RequestType::NEXT:
                controlType = PlayControlType::NEXT;
                break;
            case RequestType::PREVIOUS:
                controlType = PlayControlType::PREVIOUS;
                break;
            case RequestType::START_OVER:
                controlType = PlayControlType::START_OVER;
                break;
            case RequestType::FAST_FORWARD:
                controlType = PlayControlType::FAST_FORWARD;
                break;
            case RequestType::REWIND:
                controlType = PlayControlType::REWIND;
                break;
            case RequestType::ENABLE_REPEAT_ONE:
                controlType = PlayControlType::ENABLE_REPEAT_ONE;
                break;
            case RequestType::ENABLE_REPEAT:
                controlType = PlayControlType::ENABLE_REPEAT;
                break;
            case RequestType::DISABLE_REPEAT:
                controlType = PlayControlType::DISABLE_REPEAT;
                break;
            case RequestType::ENABLE_SHUFFLE:
                controlType = PlayControlType::ENABLE_SHUFFLE;
                break;
            case RequestType::DISABLE_SHUFFLE:
                controlType = PlayControlType::DISABLE_SHUFFLE;
                break;
            case RequestType::FAVORITE:
                controlType = PlayControlType::FAVORITE;
                break;
            case RequestType::UNFAVORITE:
                controlType = PlayControlType::UNFAVORITE;
                break;
            // internal
            case RequestType::PAUSE_RESUME_TOGGLE:
                controlType = PlayControlType::PAUSE_RESUME_TOGGLE;
                break;
            default:
                Throw("unsupportedRequestType");
        }

        ThrowIfNot(handlePlayControl(localPlayerId, controlType), "handlePlayControlFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("playerId", localPlayerId));
        return false;
    }
}

bool ExternalMediaAdapterHandler::seek(const std::string& playerId, std::chrono::milliseconds offset) {
    try {
        auto it = m_alexaToLocalPlayerIdMap.find(playerId);
        ThrowIf(it == m_alexaToLocalPlayerIdMap.end(), "invalidPlayerId");

        // call the platform media adapter
        ThrowIfNot(handleSeek(it->second, offset), "handleSeekFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "seek").d("reason", ex.what()).d("playerId", playerId));
        return false;
    }
}

bool ExternalMediaAdapterHandler::adjustSeek(const std::string& playerId, std::chrono::milliseconds deltaOffset) {
    try {
        auto it = m_alexaToLocalPlayerIdMap.find(playerId);
        ThrowIf(it == m_alexaToLocalPlayerIdMap.end(), "invalidPlayerId");

        // call the platform media adapter
        ThrowIfNot(handleAdjustSeek(it->second, deltaOffset), "handleAdjustSeekFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "adjustSeek").d("reason", ex.what()).d("playerId", playerId));
        return false;
    }
}

std::vector<aace::engine::alexa::AdapterState> ExternalMediaAdapterHandler::getAdapterStates(bool all) {
    try {
        std::vector<aace::engine::alexa::AdapterState> adapterStateList;

        for (const auto& next : m_playerInfoMap) {
            auto playerInfo = next.second;
            aace::engine::alexa::AdapterState state;

            // default session state
            state.sessionState.playerId = playerInfo.playerId;
            state.sessionState.skillToken = playerInfo.skillToken;
            state.sessionState.playbackSessionId = playerInfo.playbackSessionId;
            state.sessionState.spiVersion = playerInfo.spiVersion;

            // default playback state
            state.playbackState.playerId = playerInfo.playerId;

            if (all) {
                // get the player state from the adapter implementation
                ThrowIfNot(handleGetAdapterState(playerInfo.localPlayerId, state), "handleGetAdapterStateFailed");
            }
            adapterStateList.push_back(state);
        }

        return adapterStateList;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "getAdapterStates").d("reason", ex.what()));
        return std::vector<aace::engine::alexa::AdapterState>();
    }
}

std::chrono::milliseconds ExternalMediaAdapterHandler::getOffset(const std::string& playerId) {
    try {
        auto it = m_alexaToLocalPlayerIdMap.find(playerId);
        ThrowIf(it == m_alexaToLocalPlayerIdMap.end(), "invalidPlayerId");

        // call the platform media adapter
        return handleGetOffset(it->second);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("playerId", playerId));
        return std::chrono::milliseconds::zero();
    }
}

std::string ExternalMediaAdapterHandler::createExternalMediaPlayerEvent(
    const std::string& localPlayerId,
    const std::string& event,
    bool includePlaybackSessionId,
    std::function<void(rapidjson::Value::Object&, rapidjson::Value::AllocatorType&)> createPayload) {
    try {
        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerInfo");
        auto playerInfo = m_playerInfoMap[localPlayerId];
        return createExternalMediaPlayerEvent(playerInfo, event, includePlaybackSessionId, createPayload);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createExternalMediaPlayerEvent").d("reason", ex.what()).d("localPlayerId", localPlayerId));
        return "";
    }
}

std::string ExternalMediaAdapterHandler::createExternalMediaPlayerEvent(
    const PlayerInfo& playerInfo,
    const std::string& event,
    bool includePlaybackSessionId,
    std::function<void(rapidjson::Value::Object&, rapidjson::Value::AllocatorType&)> createPayload) {
    try {
        // create the event payload
        rapidjson::Document document(rapidjson::kObjectType);

        // create payload data
        auto payload = document.GetObject();

        // call the lamda createPayload() function
        createPayload(payload, document.GetAllocator());

        payload.AddMember(
            "playerId",
            rapidjson::Value().SetString(playerInfo.playerId.c_str(), playerInfo.playerId.length()),
            document.GetAllocator());
        payload.AddMember(
            "skillToken",
            rapidjson::Value().SetString(playerInfo.skillToken.c_str(), playerInfo.skillToken.length()),
            document.GetAllocator());

        if (includePlaybackSessionId) {
            payload.AddMember(
                "playbackSessionId",
                rapidjson::Value().SetString(
                    playerInfo.playbackSessionId.c_str(), playerInfo.playbackSessionId.length()),
                document.GetAllocator());
        }

        // create payload string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);

        return alexaClientSDK::avsCommon::avs::buildJsonEventString(
                   "ExternalMediaPlayer", event, "", buffer.GetString())
            .second;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createExternalMediaPlayerEvent").d("reason", ex.what()).d("playerId", playerInfo.playerId));
        return "";
    }
}

//
// alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface
//

bool ExternalMediaAdapterHandler::startDucking() {
    return false;
}

bool ExternalMediaAdapterHandler::stopDucking() {
    return false;
}

bool ExternalMediaAdapterHandler::setUnduckedVolume(int8_t volume) {
    try {
        ThrowIfNot(handleSetVolume(volume), "handleSetVolumeFailed");

        m_volume = volume;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaAdapterHandler::setMute(bool mute) {
    try {
        m_muted = mute;

        ThrowIfNot(handleSetMute(mute), "handleSetMuteFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("mute", mute));
        return false;
    }
}

bool ExternalMediaAdapterHandler::getSpeakerSettings(
    alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings) const {
    try {
        settings->volume = m_volume;
        settings->mute = m_muted;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type ExternalMediaAdapterHandler::getSpeakerType()
    const {
    return alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME;
}

void ExternalMediaAdapterHandler::reportDiscoveredPlayers(
    const std::vector<aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo>& discoveredPlayers) {
    try {
        auto m_discoveredPlayerSender_lock = m_discoveredPlayerSender.lock();
        ThrowIfNull(m_discoveredPlayerSender_lock, "invalidDiscoveredPlayerSender");

        // add the player info to the registered player map
        for (const auto& next : discoveredPlayers) {
            m_playerInfoMap[next.localPlayerId] = PlayerInfo(next.localPlayerId, next.spiVersion);
        }

        // used the discovered player sender to report the players
        m_discoveredPlayerSender_lock->reportDiscoveredPlayers(discoveredPlayers);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "reportDiscoveredPlayers").d("reason", ex.what()));
    }
}

bool ExternalMediaAdapterHandler::removeDiscoveredPlayer(const std::string& localPlayerId) {
    try {
        auto it = m_playerInfoMap.find(localPlayerId);
        ThrowIf(it == m_playerInfoMap.end(), "invalidLocalPlayerId");

        // remove the player form the alexa to local player id map
        m_alexaToLocalPlayerIdMap.erase(it->second.playerId);

        // remove the player info map entry
        m_playerInfoMap.erase(it);

        auto m_discoveredPlayerSender_lock = m_discoveredPlayerSender.lock();
        ThrowIfNull(m_discoveredPlayerSender_lock, "invalidDiscoveredPlayerSender");

        // notify the discovered player sender that the player has been removed
        m_discoveredPlayerSender_lock->removeDiscoveredPlayer(localPlayerId);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "removeDiscoveredPlayer").d("reason", ex.what()).d("localPlayerId", localPlayerId));
        return false;
    }
}

void ExternalMediaAdapterHandler::reportPlaybackSessionId(
    const std::string& localPlayerId,
    const std::string& sessionId) {
    try {
        ReturnIf(localPlayerId.empty());  // generic player does not track session ID
        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerInfo");
        auto it = m_playerInfoMap.find(localPlayerId);
        ThrowIf(it == m_playerInfoMap.end(), "invalidLocalPlayerId");

        m_playerInfoMap[localPlayerId].playbackSessionId = sessionId;
        AACE_INFO(LX(TAG).d("localPlayerId", localPlayerId).d("sessionId", sessionId));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("localPlayerId", localPlayerId).d("sessionId", sessionId));
    }
}

void ExternalMediaAdapterHandler::loginComplete(const std::string& localPlayerId) {
    try {
        AACE_INFO(LX(TAG).d("localPlayerId", localPlayerId));

        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerInfo");
        auto playerInfo = m_playerInfoMap[localPlayerId];
        const std::string playerId = playerInfo.playerId;

        DataPoint latencyDataPoint = m_loginDurationBuilder.withName(METRIC_EMP_RESPONSE_LATENCY).stopTimer().build();
        submitResponseLatencyMetric(
            m_metricRecorder.lock(),
            aace::engine::utils::agent::AGENT_ID_NONE,
            METRIC_REQUEST_TYPE_LOGIN,
            playerId,
            latencyDataPoint);

        auto event = createExternalMediaPlayerEvent(playerInfo, "Login");
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(
            alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_ALL, event);

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "loginComplete failed").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterHandler::logoutComplete(const std::string& localPlayerId) {
    try {
        AACE_INFO(LX(TAG).d("localPlayerId", localPlayerId));

        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerInfo");
        auto playerInfo = m_playerInfoMap[localPlayerId];
        const std::string playerId = playerInfo.playerId;

        DataPoint latencyDataPoint = m_logoutDurationBuilder.withName(METRIC_EMP_RESPONSE_LATENCY).stopTimer().build();
        submitResponseLatencyMetric(
            m_metricRecorder.lock(),
            aace::engine::utils::agent::AGENT_ID_NONE,
            METRIC_REQUEST_TYPE_LOGOUT,
            playerId,
            latencyDataPoint);

        auto event = createExternalMediaPlayerEvent(playerInfo, "Logout");
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(
            alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_ALL, event);

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "logoutComplete failed").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterHandler::playerEvent(const std::string& localPlayerId, const std::string& eventName) {
    try {
        AACE_INFO(LX(TAG).d("eventName", eventName).d("localPlayerId", localPlayerId));

        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerInfo");
        auto playerInfo = m_playerInfoMap[localPlayerId];
        const std::string playerId = playerInfo.playerId;

        submitEventCountMetric(m_metricRecorder.lock(), aace::engine::utils::agent::AGENT_ID_ALL, eventName, playerId);

        if (eventName.compare(aace::engine::alexa::PLAYBACK_SESSION_STARTED) == 0 ||
            eventName.compare(aace::engine::alexa::TRACK_CHANGED) == 0 ||
            eventName.compare(aace::engine::alexa::PLAYBACK_STARTED) == 0) {
            AACE_DEBUG(LX(TAG).d("Setting focus for localPlayerId:", localPlayerId));
            if (!setFocus(localPlayerId, true)) {
                AACE_ERROR(LX(TAG).m("setFocus(true) failed").d("localPlayerId", localPlayerId));
            }
        }

        if (eventName.compare(aace::engine::alexa::PLAYBACK_SESSION_ENDED) == 0) {
            ThrowIfNot(setFocus(localPlayerId, false), "setFocusFailed");
            if (!setFocus(localPlayerId, false)) {
                AACE_ERROR(LX(TAG).m("setFocus(false) failed").d("localPlayerId", localPlayerId));
            }
        }

        auto event = createExternalMediaPlayerEvent(
            playerInfo,
            "PlayerEvent",
            true,
            [eventName](rapidjson::Value::Object& payload, rapidjson::Value::AllocatorType& allocator) {
                payload.AddMember(
                    "eventName", rapidjson::Value().SetString(eventName.c_str(), eventName.length()), allocator);
            });
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(
            alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_ALL, event);

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "playerEvent failed").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterHandler::playerError(
    const std::string& localPlayerId,
    const std::string& errorName,
    long code,
    const std::string& description,
    bool fatal) {
    try {
        AACE_INFO(
            LX(TAG).d("errorName", errorName).d("localPlayerId", localPlayerId).d("code", code).d("fatal", fatal));

        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerInfo");
        auto playerInfo = m_playerInfoMap[localPlayerId];
        const std::string playerId = playerInfo.playerId;

        submitErrorCountMetric(m_metricRecorder.lock(), aace::engine::utils::agent::AGENT_ID_ALL, errorName, playerId);

        auto event = createExternalMediaPlayerEvent(
            playerInfo,
            "PlayerError",
            true,
            [errorName, code, description, fatal](
                rapidjson::Value::Object& payload, rapidjson::Value::AllocatorType& allocator) {
                payload.AddMember(
                    "errorName", rapidjson::Value().SetString(errorName.c_str(), errorName.length()), allocator);
                payload.AddMember("code", rapidjson::Value().SetInt64(code), allocator);
                payload.AddMember(
                    "description", rapidjson::Value().SetString(description.c_str(), description.length()), allocator);
                payload.AddMember("fatal", rapidjson::Value().SetBool(fatal), allocator);
            });
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(
            alexaClientSDK::avsCommon::avs::AgentId::AGENT_ID_ALL, event);

        // cases where we should abandon focus
        if (errorName.compare(aace::engine::alexa::UNPLAYABLE_BY_AUTHORIZATION) == 0 ||
            errorName.compare(aace::engine::alexa::UNPLAYABLE_BY_STREAM_CONCURRENCY) == 0 ||
            (errorName.compare(aace::engine::alexa::INTERNAL_ERROR) == 0 && fatal)) {
            ThrowIfNot(setFocus(localPlayerId, false), "setFocusFailed");
        }

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "playerError failed").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterHandler::doShutdown() {
    m_executor.shutdown();

    if (!m_discoveredPlayerSender.expired()) {
        m_discoveredPlayerSender.reset();
    }

    if (!m_focusHandler.expired()) {
        m_focusHandler.reset();
    }

    if (!m_metricRecorder.expired()) {
        m_metricRecorder.reset();
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
