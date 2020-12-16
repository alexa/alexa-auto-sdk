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

#include "AACE/Engine/Alexa/LocalMediaSourceEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace aace {
namespace engine {
namespace alexa {

static const uint8_t MAX_SPEAKER_VOLUME = 100;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.LocalMediaSourceEngineImpl");

static const std::string CONTENT_SELECTOR_SEPARATOR = ":";

static const std::string DEFAULT_PLAYERCOOKIE_PAYLOAD = R"(
    {
        "cookieVersion": "1.0",
        "capabilities": {}
    }
)";

LocalMediaSourceEngineImpl::LocalMediaSourceEngineImpl(
    std::shared_ptr<aace::alexa::LocalMediaSource> platformLocalMediaSource,
    const std::string& localPlayerId,
    std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender,
    std::shared_ptr<FocusHandlerInterface> focusHandler) :
        aace::engine::alexa::ExternalMediaAdapterHandler(discoveredPlayerSender, focusHandler),
        m_platformLocalMediaSource(platformLocalMediaSource),
        m_localPlayerId(localPlayerId),
        m_contentSelectorNameMap{{"frequency", ContentSelector::FREQUENCY},
                                 {"channel", ContentSelector::CHANNEL},
                                 {"preset", ContentSelector::PRESET},
                                 {"dabchannel", ContentSelector::CHANNEL}} {
}

std::shared_ptr<LocalMediaSourceEngineImpl> LocalMediaSourceEngineImpl::create(
    std::shared_ptr<aace::alexa::LocalMediaSource> platformLocalMediaSource,
    const std::string& localPlayerId,
    std::shared_ptr<DiscoveredPlayerSenderInterface> discoveredPlayerSender,
    std::shared_ptr<FocusHandlerInterface> focusHandler,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager) {
    std::shared_ptr<LocalMediaSourceEngineImpl> localMediaSourceEngineImpl = nullptr;

    try {
        ThrowIfNull(platformLocalMediaSource, "invalidPlatformLocalMediaSource");
        ThrowIf(localPlayerId.empty(), "invalidPlayerId");
        ThrowIfNull(discoveredPlayerSender, "invalidDiscoveredPlayerSender");
        ThrowIfNull(focusHandler, "invalidFocusHandler");

        // create the external media adapter engine implementation
        localMediaSourceEngineImpl = std::shared_ptr<LocalMediaSourceEngineImpl>(new LocalMediaSourceEngineImpl(
            platformLocalMediaSource, localPlayerId, discoveredPlayerSender, focusHandler));
        ThrowIfNot(
            localMediaSourceEngineImpl->initialize(messageSender, speakerManager),
            "initializeLocalMediaSourceEngineImplFailed");

        // register the platforms engine interface
        platformLocalMediaSource->setEngineInterface(localMediaSourceEngineImpl);

        return localMediaSourceEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (localMediaSourceEngineImpl != nullptr) {
            localMediaSourceEngineImpl->shutdown();
        }
        return nullptr;
    }
}

bool LocalMediaSourceEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager) {
    try {
        ThrowIfNull(messageSender, "invalidMessageSender");
        m_messageSender = messageSender;

        // initialize the adapter handler
        ThrowIfNot(initializeAdapterHandler(speakerManager), "initializeAdapterHandlerFailed");

        // report the player as discovered
        aace::alexa::ExternalMediaAdapter::DiscoveredPlayerInfo info;

        info.localPlayerId = m_localPlayerId;
        info.spiVersion = "1.0";
        info.validationMethod = VALIDATION_NONE;
        info.validationData.push_back("None");

        reportDiscoveredPlayers({info});

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleAuthorization(
    const std::vector<aace::alexa::ExternalMediaAdapter::AuthorizedPlayerInfo>& authorizedPlayerList) {
    try {
        AACE_VERBOSE(LX(TAG));

        // validate the authorized player list has exactly one player
        ThrowIf(authorizedPlayerList.size() != 1, "invalidAuthorizedPlayerList");

        // assume authorization will always return true for a registered local media source
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleAuthorization").d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleLogin(
    const std::string& localPlayerId,
    const std::string& accessToken,
    const std::string& userName,
    bool forceLogin,
    std::chrono::milliseconds tokenRefreshInterval) {
    try {
        AACE_VERBOSE(LX(TAG));

        Throw("unsupportedLocalMediaSourceOperation");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleLogin")
                       .d("reason", ex.what())
                       .d("playerId", localPlayerId)
                       .d("userName", userName)
                       .d("forceLogin", forceLogin));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleLogout(const std::string& localPlayerId) {
    try {
        AACE_VERBOSE(LX(TAG));

        Throw("unsupportedLocalMediaSourceOperation");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleLogout").d("reason", ex.what()).d("playerId", localPlayerId));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handlePlay(
    const std::string& localPlayerId,
    const std::string& playContextToken,
    int64_t index,
    std::chrono::milliseconds offset,
    bool preload,
    aace::alexa::ExternalMediaAdapter::Navigation navigation,
    const std::string& playbackSessionId,
    const std::string& skillToken) {
    try {
        AACE_VERBOSE(LX(TAG));

        // For Local Media sources localPlayerId, index, offset, preload, and navigation are currently unused

        size_t separatorIndex = playContextToken.find(CONTENT_SELECTOR_SEPARATOR);  // ":"

        std::string contentSelectorName = playContextToken.substr(0, separatorIndex);  //  i.e."frequency"
        std::transform(
            contentSelectorName.begin(),
            contentSelectorName.end(),
            contentSelectorName.begin(),
            ::tolower);  // enforce lower case

        std::unordered_map<std::string, ContentSelector>::const_iterator foundIt =
            m_contentSelectorNameMap.find(contentSelectorName);
        ThrowIfNot(foundIt != m_contentSelectorNameMap.end(), "getContentSelectorFromNameFailed");
        ContentSelector contentSelector = foundIt->second;

        std::string selectionPayload = playContextToken.substr(separatorIndex + 1);  //  i.e."98.7"

        ThrowIfNot(
            m_platformLocalMediaSource->play(contentSelector, selectionPayload), "platformMediaAdapterPlayFailed");
        // set focus on successful play
        ThrowIfNot(setFocus(m_localPlayerId, true), "setFocusFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handlePlay").d("reason", ex.what()).d("playerId", localPlayerId).d("preload", preload));
        // unset focus if playback fails
        try {
            ThrowIfNot(setFocus(m_localPlayerId, false), "setFocusFailed");
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "setFocus").d("reason", ex.what()));
        }
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handlePlayControl(
    const std::string& localPlayerId,
    aace::alexa::ExternalMediaAdapter::PlayControlType playControlType) {
    try {
        AACE_VERBOSE(LX(TAG));

        ThrowIfNot(m_platformLocalMediaSource->playControl(playControlType), "platformMediaAdapterPlayControlFailed");
        // set focus on successful RESUME control
        if (playControlType == aace::alexa::ExternalMediaAdapter::PlayControlType::RESUME)
            ThrowIfNot(setFocus(m_localPlayerId, true), "setFocusFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handlePlayControl")
                       .d("reason", ex.what())
                       .d("playerId", localPlayerId)
                       .d("playControlType", playControlType));
        if (playControlType == aace::alexa::ExternalMediaAdapter::PlayControlType::RESUME) {
            // unset focus if playback fails
            try {
                ThrowIfNot(setFocus(m_localPlayerId, false), "setFocusFailed");
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG, "setFocus").d("reason", ex.what()));
            }
        }
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleSeek(const std::string& localPlayerId, std::chrono::milliseconds offset) {
    try {
        AACE_VERBOSE(LX(TAG).d("localPlayerId", localPlayerId));

        ThrowIfNot(m_platformLocalMediaSource->seek(offset), "platformMediaAdapterSeekFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleSeek").d("reason", ex.what()).d("playerId", localPlayerId));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleAdjustSeek(
    const std::string& localPlayerId,
    std::chrono::milliseconds deltaOffset) {
    try {
        AACE_VERBOSE(LX(TAG).d("localPlayerId", localPlayerId));

        ThrowIfNot(m_platformLocalMediaSource->adjustSeek(deltaOffset), "platformMediaAdapterAdjustSeekFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "adjustSeek").d("reason", ex.what()).d("playerId", localPlayerId));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleGetAdapterState(
    const std::string& localPlayerId,
    aace::engine::alexa::AdapterState& state) {
    try {
        auto platformState = m_platformLocalMediaSource->getState();
        auto platformSource = m_platformLocalMediaSource->getSource();

        // session state
        if (platformState.sessionState.spiVersion.empty() == false) {
            state.sessionState.spiVersion = platformState.sessionState.spiVersion;
        }
        state.sessionState.playerId = getPlayerId(platformSource);
        state.sessionState.endpointId = platformState.sessionState.endpointId;
        state.sessionState.loggedIn = platformState.sessionState.loggedIn;
        state.sessionState.userName = platformState.sessionState.userName;
        state.sessionState.isGuest = platformState.sessionState.isGuest;
        state.sessionState.launched = platformState.sessionState.launched;
        state.sessionState.active = platformState.sessionState.active;
        state.sessionState.accessToken = platformState.sessionState.accessToken;
        state.sessionState.tokenRefreshInterval = platformState.sessionState.tokenRefreshInterval;

        // construct playercookie payload
        rapidjson::Document document;
        document.Parse<0>(DEFAULT_PLAYERCOOKIE_PAYLOAD.c_str());
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        for (auto next : platformState.sessionState.supportedContentSelectors) {
            switch (next) {
                case aace::alexa::LocalMediaSource::ContentSelector::FREQUENCY:
                    document["capabilities"].AddMember("playFrequency", "1.0", allocator);
                    break;
                case aace::alexa::LocalMediaSource::ContentSelector::CHANNEL:
                    document["capabilities"].AddMember("playChannel", "1.0", allocator);
                    break;
                case aace::alexa::LocalMediaSource::ContentSelector::PRESET:
                    document["capabilities"].AddMember("playPreset", "1.0", allocator);
                    break;
            }
        }

        // add dynamic pluggable capability clearlist payload
        document["capabilities"].AddMember("enableIsLaunched", "1.0", allocator);

        rapidjson::StringBuffer strbuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
        document.Accept(writer);
        state.sessionState.playerCookie = strbuf.GetString();

        // playback state
        state.playbackState.playerId = getPlayerId(platformSource);
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
        AACE_ERROR(LX(TAG, "handleGetAdapterState").d("reason", ex.what()).d("playerId", localPlayerId));
        return false;
    }
}

std::chrono::milliseconds LocalMediaSourceEngineImpl::handleGetOffset(const std::string& localPlayerId) {
    return std::chrono::milliseconds::zero();
}

std::string LocalMediaSourceEngineImpl::getPlayerId(Source source) {
    switch (source) {
        case Source::AM_RADIO:
            return "AM_RADIO";

        case Source::FM_RADIO:
            return "FM_RADIO";

        case Source::BLUETOOTH:
            return "BLUETOOTH";

        case Source::COMPACT_DISC:
            return "COMPACT_DISC";

        case Source::LINE_IN:
            return "LINE_IN";

        case Source::SATELLITE_RADIO:
            return "SATELLITE_RADIO";

        case Source::USB:
            return "USB";

        case Source::SIRIUS_XM:
            return "SIRIUS_XM";

        case Source::DAB:
            return "DAB_RADIO";

        default:
            throw("invalidLocalMediaSource");
    }
}

bool LocalMediaSourceEngineImpl::handleSetVolume(int8_t volume) {
    try {
        ThrowIfNot(
            m_platformLocalMediaSource->volumeChanged((float)volume / MAX_SPEAKER_VOLUME),
            "platformLocalMediaSourceVolumeChangedFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool LocalMediaSourceEngineImpl::handleSetMute(bool mute) {
    try {
        ThrowIfNot(
            m_platformLocalMediaSource->mutedStateChanged(
                mute ? aace::alexa::ExternalMediaAdapter::MutedState::MUTED
                     : aace::alexa::ExternalMediaAdapter::MutedState::UNMUTED),
            "platformLocalMediaSourceMutedStateChangedFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("mute", mute));
        return false;
    }
}

//
// aace::alexa::LocalMediaSourceEngineInterface
//

void LocalMediaSourceEngineImpl::onPlayerEvent(const std::string& eventName) {
    try {
        AACE_VERBOSE(LX(TAG).d("eventName", eventName));

        auto event = createExternalMediaPlayerEvent(
            m_localPlayerId,
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
        AACE_ERROR(LX(TAG, "onPlayerEvent").d("reason", ex.what()).d("eventName", eventName));
    }
}

void LocalMediaSourceEngineImpl::onPlayerError(
    const std::string& errorName,
    long code,
    const std::string& description,
    bool fatal) {
    try {
        AACE_VERBOSE(LX(TAG).d("errorName", errorName).d("code", code).d("description", description).d("fatal", fatal));

        auto event = createExternalMediaPlayerEvent(
            m_localPlayerId,
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

        auto m_messageSender_lock = m_messageSender.lock();
        ThrowIfNull(m_messageSender_lock, "invalidMessageSender");

        m_messageSender_lock->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onPlayerError")
                       .d("reason", ex.what())
                       .d("errorName", errorName)
                       .d("code", code)
                       .d("description", description)
                       .d("fatal", fatal));
    }
}

void LocalMediaSourceEngineImpl::onSetFocus(bool focusAcquire) {
    try {
        ThrowIfNot(setFocus(m_localPlayerId, focusAcquire), "setFocusFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onSetFocus").d("reason", ex.what()));
    }
}

// alexaClientSDK::avsCommon::utils::RequiresShutdown
void LocalMediaSourceEngineImpl::doShutdown() {
    AACE_VERBOSE(LX(TAG));

    if (m_platformLocalMediaSource != nullptr) {
        m_platformLocalMediaSource->setEngineInterface(nullptr);
        m_platformLocalMediaSource.reset();
    }

    m_messageSender.reset();
    ExternalMediaAdapterHandler::doShutdown();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
