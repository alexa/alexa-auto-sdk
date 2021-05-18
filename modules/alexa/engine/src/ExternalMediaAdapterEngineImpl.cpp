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

#include "AACE/Engine/Alexa/ExternalMediaAdapterEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/Metrics/Metrics.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;

static const uint8_t MAX_SPEAKER_VOLUME = 100;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.ExternalMediaAdapterEngineImpl");

// external media player agent constant
static const std::string EXTERNAL_MEDIA_PLAYER_AGENT = "Alexa Auto SDK";

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "ExternalMediaAdapterEngineImpl";

/// Counter metrics for ExternalMediaPlayer Platform APIs
static const std::string METRIC_EXTERNALMEDIAPLAYER_LOGIN = "Login";
static const std::string METRIC_EXTERNALMEDIAPLAYER_LOGOUT = "Logout";
static const std::string METRIC_EXTERNALMEDIAPLAYER_PLAY = "Play";
static const std::string METRIC_EXTERNALMEDIAPLAYER_PLAY_CONTROL = "PlayControl";
static const std::string METRIC_EXTERNALMEDIAPLAYER_SEEK = "Seek";
static const std::string METRIC_EXTERNALMEDIAPLAYER_ADJUST_SEEK = "AdjustSeek";
static const std::string METRIC_EXTERNALMEDIAPLAYER_AUTHORIZE = "Authorize";
static const std::string METRIC_EXTERNALMEDIAPLAYER_GET_OFFSET = "GetOffset";
static const std::string METRIC_EXTERNALMEDIAPLAYER_VOLUME_CHANGED = "VolumeChanged";
static const std::string METRIC_EXTERNALMEDIAPLAYER_MUTED_STATE_CHANGED = "MutedStateChanged";
static const std::string METRIC_EXTERNALMEDIAPLAYER_REPORT_DISCOVERED_PLAYERS = "ReportDiscoveredPlayers";
static const std::string METRIC_EXTERNALMEDIAPLAYER_REQUEST_TOKEN = "RequestToken";
static const std::string METRIC_EXTERNALMEDIAPLAYER_LOGIN_COMPLETE = "LoginComplete";
static const std::string METRIC_EXTERNALMEDIAPLAYER_LOGOUT_COMPLETE = "LogoutComplete";
static const std::string METRIC_EXTERNALMEDIAPLAYER_PLAYER_EVENT = "PlayerEvent";
static const std::string METRIC_EXTERNALMEDIAPLAYER_PLAYER_ERROR = "PlayerError";
static const std::string METRIC_EXTERNALMEDIAPLAYER_SET_FOCUS = "SetFocus";
static const std::string METRIC_EXTERNALMEDIAPLAYER_REMOVE_DISCOVERED_PLAYER = "RemoveDiscoveredPlayer";

ExternalMediaAdapterEngineImpl::ExternalMediaAdapterEngineImpl(
    std::shared_ptr<aace::alexa::ExternalMediaAdapter> platformMediaAdapter,
    std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender,
    std::shared_ptr<FocusHandlerInterface> focusHandler) :
        aace::engine::alexa::ExternalMediaAdapterHandler(discoveredPlayerSender, focusHandler),
        m_platformMediaAdapter(platformMediaAdapter) {
}

std::shared_ptr<ExternalMediaAdapterEngineImpl> ExternalMediaAdapterEngineImpl::create(
    std::shared_ptr<aace::alexa::ExternalMediaAdapter> platformMediaAdapter,
    std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender,
    std::shared_ptr<FocusHandlerInterface> focusHandler,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager) {
    std::shared_ptr<ExternalMediaAdapterEngineImpl> externalMediaAdapterEngineImpl = nullptr;

    try {
        ThrowIfNull(platformMediaAdapter, "invalidPlatformMediaAdapter");
        ThrowIfNull(discoveredPlayerSender, "invalidDiscoveredPlayerSender");
        ThrowIfNull(focusHandler, "invalidFocusHandler");

        // create the external media adapter engine implementation
        externalMediaAdapterEngineImpl = std::shared_ptr<ExternalMediaAdapterEngineImpl>(
            new ExternalMediaAdapterEngineImpl(platformMediaAdapter, discoveredPlayerSender, focusHandler));
        ThrowIfNot(
            externalMediaAdapterEngineImpl->initialize(messageSender, speakerManager),
            "initializeExternalMediaAdapterEngineImplFailed");

        // register the platforms engine interface
        platformMediaAdapter->setEngineInterface(externalMediaAdapterEngineImpl);

        return externalMediaAdapterEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (externalMediaAdapterEngineImpl != nullptr) {
            externalMediaAdapterEngineImpl->shutdown();
        }
        return nullptr;
    }
}

bool ExternalMediaAdapterEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager) {
    try {
        ThrowIfNull(messageSender, "invalidMessageSender");
        m_messageSender = messageSender;

        // initialize the adapter handler
        ThrowIfNot(initializeAdapterHandler(speakerManager), "initializeAdapterHandlerFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaAdapterEngineImpl::handleAuthorization(
    const std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo>& authorizedPlayerList) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "handleAuthorization", {METRIC_EXTERNALMEDIAPLAYER_AUTHORIZE});
    try {
        ThrowIfNot(m_platformMediaAdapter->authorize(authorizedPlayerList), "platformMediaAdapterAuthorizeFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleAuthorization").d("reason", ex.what()));
        return false;
    }
}

bool ExternalMediaAdapterEngineImpl::handleLogin(
    const std::string& localPlayerId,
    const std::string& accessToken,
    const std::string& userName,
    bool forceLogin,
    std::chrono::milliseconds tokenRefreshInterval) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "handleLogin", {METRIC_EXTERNALMEDIAPLAYER_LOGIN});
    try {
        ThrowIfNot(
            m_platformMediaAdapter->login(localPlayerId, accessToken, userName, forceLogin, tokenRefreshInterval),
            "platformMediaAdapterLoginFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleLogin").d("reason", ex.what()).d("localPlayerId", localPlayerId));
        return false;
    }
}

bool ExternalMediaAdapterEngineImpl::handleLogout(const std::string& localPlayerId) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "handleLogout", {METRIC_EXTERNALMEDIAPLAYER_LOGOUT});
    try {
        ThrowIfNot(m_platformMediaAdapter->logout(localPlayerId), "platformMediaAdapterLogoutFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleLogout").d("reason", ex.what()).d("localPlayerId", localPlayerId));
        return false;
    }
}

bool ExternalMediaAdapterEngineImpl::handlePlay(
    const std::string& localPlayerId,
    const std::string& playContextToken,
    int64_t index,
    std::chrono::milliseconds offset,
    bool preload,
    aace::alexa::ExternalMediaAdapter::Navigation navigation,
    const std::string& playbackSessionId,
    const std::string& skillToken) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "handlePlay", {METRIC_EXTERNALMEDIAPLAYER_PLAY});
    try {
        // note: playRequestor.id not needed by client for now
        ThrowIfNot(
            m_platformMediaAdapter->play(
                localPlayerId, playContextToken, index, offset, preload, navigation, playbackSessionId, skillToken),
            "platformMediaAdapterPlayFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(
            LX(TAG, "handlePlay").d("reason", ex.what()).d("localPlayerId", localPlayerId).d("preload", preload));
        return false;
    }
}

bool ExternalMediaAdapterEngineImpl::handlePlayControl(
    const std::string& localPlayerId,
    aace::alexa::ExternalMediaAdapter::PlayControlType playControlType) {
    std::stringstream type;
    type << playControlType;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "handlePlayControl", {METRIC_EXTERNALMEDIAPLAYER_PLAY_CONTROL, type.str()});
    try {
        ThrowIfNot(
            m_platformMediaAdapter->playControl(localPlayerId, playControlType),
            "platformMediaAdapterPlayControlFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handlePlayControl")
                       .d("reason", ex.what())
                       .d("localPlayerId", localPlayerId)
                       .d("controlType", playControlType));
        return false;
    }
}

bool ExternalMediaAdapterEngineImpl::handleSeek(const std::string& localPlayerId, std::chrono::milliseconds offset) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "handleSeek", {METRIC_EXTERNALMEDIAPLAYER_SEEK});
    try {
        ThrowIfNot(m_platformMediaAdapter->seek(localPlayerId, offset), "platformMediaAdapterSeekFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleSeek").d("reason", ex.what()).d("localPlayerId", localPlayerId));
        return false;
    }
}

bool ExternalMediaAdapterEngineImpl::handleAdjustSeek(
    const std::string& localPlayerId,
    std::chrono::milliseconds deltaOffset) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "handleAdjustSeek", {METRIC_EXTERNALMEDIAPLAYER_ADJUST_SEEK});
    try {
        ThrowIfNot(
            m_platformMediaAdapter->adjustSeek(localPlayerId, deltaOffset), "platformMediaAdapterAdjustSeekFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "adjustSeek").d("reason", ex.what()).d("localPlayerId", localPlayerId));
        return false;
    }
}

bool ExternalMediaAdapterEngineImpl::handleGetAdapterState(
    const std::string& localPlayerId,
    aace::engine::alexa::AdapterState& state) {
    try {
        aace::alexa::ExternalMediaAdapter::ExternalMediaAdapterState platformState;

        // get the external media adapter state from the platform interface
        ThrowIfNot(
            m_platformMediaAdapter->getState(localPlayerId, platformState),
            "getPlatformExternalMediaAdapterStateFailed");

        // session state
        if (platformState.sessionState.spiVersion.empty() == false) {
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
        state.playbackState.repeatOneEnabled = platformState.playbackState.repeatOneEnabled;
        state.playbackState.favorites =
            static_cast<aace::engine::alexa::Favorites>(platformState.playbackState.favorites);
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
        state.playbackState.mediaType =
            static_cast<aace::engine::alexa::MediaType>(platformState.playbackState.mediaType);
        state.playbackState.duration = platformState.playbackState.duration;

        // convert AAC SupportedPlaybackOperation to AVS SupportedPlaybackOperation
        using avsSupportedPlaybackOperation = aace::engine::alexa::SupportedPlaybackOperation;

        for (auto nextOp : platformState.playbackState.supportedOperations) {
            switch (nextOp) {
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PLAY:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::PLAY);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PAUSE:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::PAUSE);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::STOP:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::STOP);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::NEXT:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::NEXT);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PREVIOUS:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::PREVIOUS);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::START_OVER:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::START_OVER);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::FAST_FORWARD:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::FAST_FORWARD);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::REWIND:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::REWIND);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::ENABLE_REPEAT);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT_ONE:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::ENABLE_REPEAT_ONE);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::DISABLE_REPEAT:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::DISABLE_REPEAT);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_SHUFFLE:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::ENABLE_SHUFFLE);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::DISABLE_SHUFFLE:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::DISABLE_SHUFFLE);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::FAVORITE:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::FAVORITE);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::UNFAVORITE:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::UNFAVORITE);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::SEEK:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::SEEK);
                    break;
                case aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ADJUST_SEEK:
                    state.playbackState.supportedOperations.insert(avsSupportedPlaybackOperation::ADJUST_SEEK);
                    break;
                default:
                    AACE_VERBOSE(LX(TAG).m("Unexpected SupportedPlaybackOperation"));
                    break;
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleGetAdapterState").d("reason", ex.what()).d("localPlayerId", localPlayerId));
        return false;
    }
}

std::chrono::milliseconds ExternalMediaAdapterEngineImpl::handleGetOffset(const std::string& localPlayerId) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "handleGetOffset", {METRIC_EXTERNALMEDIAPLAYER_GET_OFFSET});
    try {
        // get the external media adapter offset from the platform interface
        return m_platformMediaAdapter->getOffset(localPlayerId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::chrono::milliseconds::zero();
    }
}

bool ExternalMediaAdapterEngineImpl::handleSetVolume(int8_t volume) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "handleSetVolume", {METRIC_EXTERNALMEDIAPLAYER_VOLUME_CHANGED});
    try {
        ThrowIfNot(
            m_platformMediaAdapter->volumeChanged((float)volume / MAX_SPEAKER_VOLUME),
            "platformMediaAdapterVolumeChangedFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("volume", volume));
        return false;
    }
}

bool ExternalMediaAdapterEngineImpl::handleSetMute(bool mute) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "handleSetMute", {METRIC_EXTERNALMEDIAPLAYER_MUTED_STATE_CHANGED});
    try {
        ThrowIfNot(
            m_platformMediaAdapter->mutedStateChanged(
                mute ? aace::alexa::ExternalMediaAdapter::MutedState::MUTED
                     : aace::alexa::ExternalMediaAdapter::MutedState::UNMUTED),
            "platformMediaAdapterMutedStateChangedFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("mute", mute));
        return false;
    }
}

//
// aace::alexa::ExternalMediaAdapterEngineInterface
//

void ExternalMediaAdapterEngineImpl::onReportDiscoveredPlayers(
    const std::vector<DiscoveredPlayerInfo>& discoveredPlayers) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onReportDiscoveredPlayers",
        {METRIC_EXTERNALMEDIAPLAYER_REPORT_DISCOVERED_PLAYERS});
    try {
        reportDiscoveredPlayers(discoveredPlayers);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onReportDiscoveredPlayers").d("reason", ex.what()));
    }
}

void ExternalMediaAdapterEngineImpl::onRequestToken(const std::string& localPlayerId) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onRequestToken", {METRIC_EXTERNALMEDIAPLAYER_REQUEST_TOKEN});
    try {
        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerId");

        auto event = createExternalMediaPlayerEvent(localPlayerId, "RequestToken");
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event);

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onRequestToken").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterEngineImpl::onLoginComplete(const std::string& localPlayerId) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onLoginComplete", {METRIC_EXTERNALMEDIAPLAYER_LOGIN_COMPLETE});
    try {
        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerId");

        auto event = createExternalMediaPlayerEvent(localPlayerId, "Login");
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event);

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onLoginComplete").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterEngineImpl::onLogoutComplete(const std::string& localPlayerId) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onLogoutComplete", {METRIC_EXTERNALMEDIAPLAYER_LOGOUT_COMPLETE});
    try {
        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerId");

        auto event = createExternalMediaPlayerEvent(localPlayerId, "Logout");
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event);

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onLogoutComplete").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterEngineImpl::onPlayerEvent(const std::string& localPlayerId, const std::string& eventName) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onPlayerEvent", {METRIC_EXTERNALMEDIAPLAYER_PLAYER_EVENT, eventName});
    try {
        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerId");
        // player has begun playing, acquire focus
        if (eventName.compare(aace::engine::alexa::PLAYBACK_STARTED) == 0) {
            AACE_VERBOSE(LX(TAG, "onPlayerEvent").d("Setting focus for localPlayerId:", localPlayerId));
            ThrowIfNot(setFocus(localPlayerId, true), "setFocusFailed");
        }

        aace::alexa::ExternalMediaAdapter::ExternalMediaAdapterState platformState;
        // when the session starts or track changes, we need to check if the app is already playing externally
        if (eventName.compare(aace::engine::alexa::PLAYBACK_SESSION_STARTED) == 0 ||
            eventName.compare(aace::engine::alexa::TRACK_CHANGED) == 0) {
            // get the external media adapter state from the platform interface
            try {
                ThrowIfNot(m_platformMediaAdapter->getState(localPlayerId, platformState), "getState failed");
                // player is already playing, acquire focus
                if (platformState.playbackState.state.compare(aace::engine::alexa::PLAYING) == 0) {
                    AACE_VERBOSE(LX(TAG, "onPlayerEvent").d("Setting focus for localPlayerId:", localPlayerId));
                    ThrowIfNot(setFocus(localPlayerId, true), "setFocusFailed");
                }
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG, "onPlayerEvent")
                               .d("Unable to set player focus", ex.what())
                               .d("localPlayerId", localPlayerId));
            }
        }

        if (eventName.compare(aace::engine::alexa::PLAYBACK_SESSION_ENDED) == 0) {
            // unsetFocus
            ThrowIfNot(setFocus(localPlayerId, false), "setFocusFailed");
        }

        auto event = createExternalMediaPlayerEvent(
            localPlayerId,
            "PlayerEvent",
            true,
            [eventName](rapidjson::Value::Object& payload, rapidjson::Value::AllocatorType& allocator) {
                payload.AddMember(
                    "eventName", rapidjson::Value().SetString(eventName.c_str(), eventName.length()), allocator);
            });
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event);

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onPlayerEvent").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterEngineImpl::onPlayerError(
    const std::string& localPlayerId,
    const std::string& errorName,
    long code,
    const std::string& description,
    bool fatal) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onPlayerError",
        {METRIC_EXTERNALMEDIAPLAYER_PLAYER_ERROR, errorName, std::to_string(code)});
    try {
        ThrowIfNot(validatePlayer(localPlayerId), "invalidPlayerId");

        auto event = createExternalMediaPlayerEvent(
            localPlayerId,
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
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event);

        // cases where we should abandon focus
        if (errorName.compare(aace::engine::alexa::UNPLAYABLE_BY_AUTHORIZATION) == 0 ||
            errorName.compare(aace::engine::alexa::UNPLAYABLE_BY_STREAM_CONCURRENCY) == 0 ||
            errorName.compare(aace::engine::alexa::INTERNAL_ERROR) == 0) {
            ThrowIfNot(setFocus(localPlayerId, false), "setFocusFailed");
        }

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onPlayerError").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterEngineImpl::onSetFocus(const std::string& localPlayerId) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onSetFocus", {METRIC_EXTERNALMEDIAPLAYER_SET_FOCUS});
    try {
        ThrowIfNot(setFocus(localPlayerId, true), "setFocusFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onSetFocus").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

void ExternalMediaAdapterEngineImpl::onRemoveDiscoveredPlayer(const std::string& localPlayerId) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "onRemoveDiscoveredPlayer", {METRIC_EXTERNALMEDIAPLAYER_REMOVE_DISCOVERED_PLAYER});
    try {
        ThrowIfNot(removeDiscoveredPlayer(localPlayerId), "removeDiscoveredPlayerFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onRemoveDiscoveredPlayer").d("reason", ex.what()).d("localPlayerId", localPlayerId));
    }
}

//
// alexaClientSDK::avsCommon::utils::RequiresShutdown
//

void ExternalMediaAdapterEngineImpl::doShutdown() {
    if (m_platformMediaAdapter != nullptr) {
        m_platformMediaAdapter->setEngineInterface(nullptr);
        m_platformMediaAdapter.reset();
    }

    m_messageSender.reset();
    ExternalMediaAdapterHandler::doShutdown();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
