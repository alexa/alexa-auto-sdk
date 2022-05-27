/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Alexa/LocalMediaSourceHandler.h"

#include <AASB/Message/Alexa/LocalMediaSource/AdjustSeekMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/GetStateMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/LocalMediaSourceState.h>
#include <AASB/Message/Alexa/LocalMediaSource/MutedStateChangedMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/PlaybackState.h>
#include <AASB/Message/Alexa/LocalMediaSource/PlayControlMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/PlayMessage.h>
#include "AASB/Message/Alexa/LocalMediaSource/PlayerErrorMessage.h"
#include "AASB/Message/Alexa/LocalMediaSource/PlayerEventMessage.h"
#include <AASB/Message/Alexa/LocalMediaSource/SeekMessage.h>
#include <AASB/Message/Alexa/LocalMediaSource/SessionState.h>
#include <AASB/Message/Alexa/LocalMediaSource/VolumeChangedMessage.h>

// C++ Standard Library
#include <chrono>
#include <sstream>
#include <string>
#include <unordered_map>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace {
struct EnumHash {
    template <typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

using namespace aasb::message::alexa::localMediaSource;
using namespace aasb::message::alexa::externalMediaAdapter;
using namespace aasb::message::alexa;

const std::unordered_map<Source, std::string, EnumHash> LocalMediaSourceToString = {
    {Source::BLUETOOTH, "Local BLUETOOTH"},
    {Source::USB, "Local USB"},
    {Source::FM_RADIO, "Local FM_RADIO"},
    {Source::AM_RADIO, "Local AM_RADIO"},
    {Source::SATELLITE_RADIO, "Local SATELLITE_RADIO"},
    {Source::LINE_IN, "Local LINE_IN"},
    {Source::COMPACT_DISC, "Local COMPACT_DISC"},
    {Source::SIRIUS_XM, "Local SIRIUS_XM"},
    {Source::DAB, "Local DAB"},
    {Source::DEFAULT, "Local DEFAULT"}};

const std::unordered_map<Source, MediaType, EnumHash> LocalMediaSourceToType = {
    {Source::BLUETOOTH, MediaType::OTHER},
    {Source::USB, MediaType::OTHER},
    {Source::FM_RADIO, MediaType::STATION},
    {Source::AM_RADIO, MediaType::STATION},
    {Source::SATELLITE_RADIO, MediaType::STATION},
    {Source::LINE_IN, MediaType::OTHER},
    {Source::COMPACT_DISC, MediaType::TRACK},
    {Source::SIRIUS_XM, MediaType::STATION},
    {Source::DAB, MediaType::STATION},
    {Source::DEFAULT, MediaType::OTHER}};

static const std::string PLAYBACK_SESSION_STARTED_EVENT = "PlaybackSessionStarted";
static const std::string PLAYBACK_SESSION_ENDED_EVENT = "PlaybackSessionEnded";
static const std::string PLAYBACK_STARTED_EVENT = "PlaybackStarted";
static const std::string PLAYBACK_STOPPED_EVENT = "PlaybackStopped";

static const std::string PLAYING_STATE = "PLAYING";
static const std::string PAUSED_STATE = "PAUSED";
static const std::string STOPPED_STATE = "STOPPED";

}  // namespace

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocalMediaSourceHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LocalMediaSourceHandler::LocalMediaSourceHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    Source source,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    m_source = source;
    auto mediaSource = LocalMediaSourceToString.find(source);
    if (mediaSource != LocalMediaSourceToString.end()) {
        m_sourceMediaProvider = mediaSource->second;
    }

    auto mediaType = LocalMediaSourceToType.find(source);
    if (mediaType != LocalMediaSourceToType.end()) {
        m_sourceMediaType = mediaType->second;
    }
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr) && (m_messageBroker != nullptr));
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> LocalMediaSourceHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> LocalMediaSourceHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void LocalMediaSourceHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to AdjustSeek Messages for source: " + m_sourceMediaProvider);
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAdjustSeekMessage(message); },
        AdjustSeekMessage::topic(),
        AdjustSeekMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to Seek Messages for source: " + m_sourceMediaProvider);
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSeekMessage(message); }, SeekMessage::topic(), SeekMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetState Messages for source: " + m_sourceMediaProvider);
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetStateMessage(message); },
        GetStateMessage::topic(),
        GetStateMessage::action());

    log(logger::LoggerHandler::Level::INFO,
        "Subscribe to MutedStateChanged Messages for source: " + m_sourceMediaProvider);
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleMutedStateChangedMessage(message); },
        MutedStateChangedMessage::topic(),
        MutedStateChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to VolumeChanged Messages for source: " + m_sourceMediaProvider);
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleVolumeChangedMessage(message); },
        VolumeChangedMessage::topic(),
        VolumeChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to PlayControl Messages for source: " + m_sourceMediaProvider);
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePlayControlMessage(message); },
        PlayControlMessage::topic(),
        PlayControlMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to Play Messages for source: " + m_sourceMediaProvider);
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePlayMessage(message); }, PlayMessage::topic(), PlayMessage::action());
}

void LocalMediaSourceHandler::handleAdjustSeekMessage(const std::string& message) {
    AdjustSeekMessage msg = json::parse(message);
    if (msg.payload.source == m_source) {
        log(logger::LoggerHandler::Level::INFO, "Received AdjustSeekMessage for source: " + m_sourceMediaProvider);
        adjustSeek(std::chrono::milliseconds(msg.payload.deltaOffset));
    }
}

void LocalMediaSourceHandler::handleSeekMessage(const std::string& message) {
    SeekMessage msg = json::parse(message);
    if (msg.payload.source == m_source) {
        log(logger::LoggerHandler::Level::INFO, "Received SeekMessage for source: " + m_sourceMediaProvider);
        seek(std::chrono::milliseconds(msg.payload.offset));
    }
}

void LocalMediaSourceHandler::handleGetStateMessage(const std::string& message) {
    GetStateMessage msg = json::parse(message);
    if (msg.payload.source == m_source) {
        log(logger::LoggerHandler::Level::INFO, "Received GetStateMessage for source: " + m_sourceMediaProvider);
        // publish reply message with state information for the source
        publishGetStateReply(msg.header.id);
    }
}

void LocalMediaSourceHandler::handleMutedStateChangedMessage(const std::string& message) {
    MutedStateChangedMessage msg = json::parse(message);
    if (msg.payload.source == m_source) {
        log(logger::LoggerHandler::Level::INFO,
            "Received MutedStateChangedMessage for source: " + m_sourceMediaProvider);
        mutedStateChanged(msg.payload.state);
    }
}

void LocalMediaSourceHandler::handleVolumeChangedMessage(const std::string& message) {
    VolumeChangedMessage msg = json::parse(message);
    if (msg.payload.source == m_source) {
        log(logger::LoggerHandler::Level::INFO, "Received VolumeChangedMessage for source: " + m_sourceMediaProvider);
        volumeChanged(msg.payload.volume);
    }
}

void LocalMediaSourceHandler::handlePlayControlMessage(const std::string& message) {
    PlayControlMessage msg = json::parse(message);
    if (msg.payload.source == m_source) {
        log(logger::LoggerHandler::Level::INFO, "Received PlayControlMessage for source: " + m_sourceMediaProvider);
        playControl(msg.payload.controlType);
    }
}

void LocalMediaSourceHandler::handlePlayMessage(const std::string& message) {
    PlayMessage msg = json::parse(message);
    if (msg.payload.source == m_source) {
        log(logger::LoggerHandler::Level::INFO, "Received PlayMessage for source: " + m_sourceMediaProvider);
        play(msg.payload.contentSelectorType, msg.payload.payload, msg.payload.sessionId);
    }
}

void LocalMediaSourceHandler::publishGetStateReply(const std::string& messageId) {
    GetStateMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = messageId;
    replyMsg.payload.state = getState();
    m_messageBroker->publish(replyMsg.toString());
    log(logger::LoggerHandler::Level::INFO, "Published GetStateMessageReply");
}

void LocalMediaSourceHandler::playerEvent(const std::string& eventName, const std::string& sessionId) {
    PlayerEventMessage msg;

    msg.payload.source = m_source;
    msg.payload.eventName = eventName;
    msg.payload.sessionId = sessionId;

    m_messageBroker->publish(msg.toString());
}

void LocalMediaSourceHandler::playerError(
    const std::string& errorName,
    long code,
    const std::string& description,
    bool fatal,
    const std::string& sessionId) {
    PlayerErrorMessage msg;

    msg.payload.source = m_source;
    msg.payload.errorName = errorName;
    msg.payload.code = code;
    msg.payload.description = description;
    msg.payload.fatal = fatal;
    msg.payload.sessionId = sessionId;

    m_messageBroker->publish(msg.toString());
}

void LocalMediaSourceHandler::play(
    ContentSelector contentSelectorType,
    const std::string& payload,
    const std::string& sessionId) {
    std::stringstream ss;
    ss << "play:source=" << m_sourceMediaProvider
       << " contentSelectorType=" << convertContentSelectorToString(contentSelectorType) << " payload=" << payload
       << " sessionId=" << sessionId;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    m_localMediaSourceStateMap[m_source] = PLAYING_STATE;
    m_sessionId = sessionId;
    if (auto activity = m_activity.lock()) {
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine(m_sourceMediaProvider, "play:", payload);
                console->printRuler();
            }
        });
    }
    playerEvent(PLAYBACK_SESSION_STARTED_EVENT, m_sessionId);
    playerEvent(PLAYBACK_STARTED_EVENT, m_sessionId);
}

void LocalMediaSourceHandler::playControl(PlayControlType controlType) {
    std::stringstream ss;
    ss << "playControl:source:" << m_sourceMediaProvider
       << " controlType=" << convertPlayControlTypeToString(controlType);
    switch (controlType) {
        case PlayControlType::RESUME:
            m_localMediaSourceStateMap[m_source] = PLAYING_STATE;
            if (m_sessionId.empty()) {
                // This is an example. A random UUID unique to every local media source session is expected here
                // An actual implementation can use its preferred way to generate the UUID
                m_sessionId = "881f70f9-7309-43f7-a559-c378fa43a19a";
            }
            playerEvent(PLAYBACK_SESSION_STARTED_EVENT, m_sessionId);
            playerEvent(PLAYBACK_STARTED_EVENT, m_sessionId);
            break;
        case PlayControlType::PAUSE:
            m_localMediaSourceStateMap[m_source] = PAUSED_STATE;
            playerEvent(PLAYBACK_STOPPED_EVENT, m_sessionId);
            break;
        case PlayControlType::STOP:
            m_localMediaSourceStateMap[m_source] = STOPPED_STATE;
            playerEvent(PLAYBACK_STOPPED_EVENT, m_sessionId);
            playerEvent(PLAYBACK_SESSION_ENDED_EVENT, m_sessionId);
            m_sessionId = "";  // Reset the session Id
            break;
        default:
            // The sample app does not handle the other play control operations,
            // but the actual implementation should.
            break;
    }
    log(logger::LoggerHandler::Level::INFO, ss.str());
    if (auto activity = m_activity.lock()) {
        std::stringstream s;
        s << convertPlayControlTypeToString(controlType);
        auto text = s.str();
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine(m_sourceMediaProvider, "play control:", text);
                console->printRuler();
            }
        });
    }
}

void LocalMediaSourceHandler::seek(std::chrono::milliseconds offset) {
    log(logger::LoggerHandler::Level::INFO, "seek:offset=" + std::to_string(offset.count()));
    if (auto activity = m_activity.lock()) {
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine(m_sourceMediaProvider, "seek:", offset.count());
                console->printRuler();
            }
        });
    }
}

void LocalMediaSourceHandler::adjustSeek(std::chrono::milliseconds deltaOffset) {
    log(logger::LoggerHandler::Level::INFO, "adjustSeek:deltaOffset=" + std::to_string(deltaOffset.count()));
    if (auto activity = m_activity.lock()) {
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine(m_sourceMediaProvider, "adjust seek:", deltaOffset.count());
                console->printRuler();
            }
        });
    }
}

LocalMediaSourceState LocalMediaSourceHandler::getState() {
    auto state = LocalMediaSourceState{};

    // default session state attributes
    // This is an example, OEM's actual implementation should report the real session state
    state.sessionState.endpointId = "";
    state.sessionState.loggedIn = false;
    state.sessionState.userName = "";
    state.sessionState.isGuest = false;
    state.sessionState.launched = true;
    state.sessionState.active = false;
    state.sessionState.accessToken = "";
    state.sessionState.tokenRefreshInterval = 0;
    state.sessionState.spiVersion = "1.0";

    // default playback state attributes
    // This is an example, OEM's actual implementation should report the real playback metadata
    state.playbackState.trackOffset = 0, state.playbackState.shuffleEnabled = false;
    state.playbackState.repeatEnabled = false;
    state.playbackState.favorites = Favorites::NOT_RATED;
    state.playbackState.type = "ExternalMediaPlayerMusicItem";
    state.playbackState.playbackSource = "";
    state.playbackState.playbackSourceId = "";
    state.playbackState.trackName = "";
    state.playbackState.trackId = "";
    state.playbackState.trackNumber = "";
    state.playbackState.artistName = "";
    state.playbackState.artistId = "";
    state.playbackState.albumName = "";
    state.playbackState.albumId = "";
    state.playbackState.tinyURL = "";
    state.playbackState.smallURL = "";
    state.playbackState.mediumURL = "";
    state.playbackState.largeURL = "";
    state.playbackState.coverId = "";
    state.playbackState.mediaProvider = "";
    state.playbackState.mediaType = m_sourceMediaType;
    state.playbackState.duration = 0;

    // Supported operations and supported content selectors based on source this handler instance supports
    std::vector<SupportedPlaybackOperation> supportedOperations{};
    std::vector<ContentSelector> supportedContentSelectors{};

    supportedOperations.insert(
        supportedOperations.begin(),
        {SupportedPlaybackOperation::PLAY, SupportedPlaybackOperation::PAUSE, SupportedPlaybackOperation::STOP});
    switch (m_source) {
        case Source::DAB:
            supportedContentSelectors.insert(supportedContentSelectors.begin(), {ContentSelector::CHANNEL});
            break;
        case Source::SATELLITE_RADIO:
            break;
        case Source::AM_RADIO:
        case Source::FM_RADIO:
            supportedContentSelectors.insert(
                supportedContentSelectors.begin(), {ContentSelector::FREQUENCY, ContentSelector::PRESET});
            break;
        case Source::SIRIUS_XM:
            supportedContentSelectors.insert(
                supportedContentSelectors.begin(), {ContentSelector::PRESET, ContentSelector::CHANNEL});
            break;
        case Source::BLUETOOTH:
        case Source::USB:
        case Source::LINE_IN:
            supportedOperations.insert(
                supportedOperations.end(),
                {SupportedPlaybackOperation::FAVORITE,
                 SupportedPlaybackOperation::UNFAVORITE,
                 SupportedPlaybackOperation::NEXT,
                 SupportedPlaybackOperation::PREVIOUS,
                 SupportedPlaybackOperation::ENABLE_SHUFFLE,
                 SupportedPlaybackOperation::DISABLE_SHUFFLE,
                 SupportedPlaybackOperation::ENABLE_REPEAT_ONE,
                 SupportedPlaybackOperation::ENABLE_REPEAT,
                 SupportedPlaybackOperation::DISABLE_REPEAT,
                 SupportedPlaybackOperation::SEEK,
                 SupportedPlaybackOperation::ADJUST_SEEK,
                 SupportedPlaybackOperation::START_OVER,
                 SupportedPlaybackOperation::FAST_FORWARD,
                 SupportedPlaybackOperation::REWIND});
            break;
        case Source::COMPACT_DISC:
            supportedOperations.insert(
                supportedOperations.end(),
                {SupportedPlaybackOperation::NEXT,
                 SupportedPlaybackOperation::PREVIOUS,
                 SupportedPlaybackOperation::ENABLE_SHUFFLE,
                 SupportedPlaybackOperation::DISABLE_SHUFFLE,
                 SupportedPlaybackOperation::ENABLE_REPEAT_ONE,
                 SupportedPlaybackOperation::ENABLE_REPEAT,
                 SupportedPlaybackOperation::DISABLE_REPEAT,
                 SupportedPlaybackOperation::SEEK,
                 SupportedPlaybackOperation::ADJUST_SEEK,
                 SupportedPlaybackOperation::START_OVER,
                 SupportedPlaybackOperation::FAST_FORWARD,
                 SupportedPlaybackOperation::REWIND});
            break;
        case Source::DEFAULT:
            supportedOperations.insert(
                supportedOperations.end(),
                {SupportedPlaybackOperation::NEXT,
                 SupportedPlaybackOperation::PREVIOUS,
                 SupportedPlaybackOperation::PLAY,
                 SupportedPlaybackOperation::PAUSE});
            break;
    }

    state.playbackState.supportedOperations = supportedOperations;

    state.sessionState.supportedContentSelectors = supportedContentSelectors;

    state.playbackState.state = m_localMediaSourceStateMap.find(m_source)->second;

    return state;
}

void LocalMediaSourceHandler::volumeChanged(float volume) {
    log(logger::LoggerHandler::Level::INFO,
        " volumeChanged:m_sourceMediaProvider=" + m_sourceMediaProvider +
            " volumeChanged:volume=" + std::to_string(volume));
}

void LocalMediaSourceHandler::mutedStateChanged(MutedState mute) {
    std::string muted = "not muted";
    if (mute == MutedState::MUTED) muted = "muted";
    log(logger::LoggerHandler::Level::INFO,
        " mutedStateChanged:m_sourceMediaProvider=" + m_sourceMediaProvider + " mutedStateChanged:mute=" + muted);
}

void LocalMediaSourceHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "LocalMediaSourceHandler", message);
}

void LocalMediaSourceHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

std::string LocalMediaSourceHandler::convertContentSelectorToString(ContentSelector contentSelectorType) {
    switch (contentSelectorType) {
        case ContentSelector::CHANNEL:
            return "CHANNEL";
        case ContentSelector::FREQUENCY:
            return "FREQUENCY";
        case ContentSelector::PRESET:
            return "PRESET";
    }
    // invalid content selector
    log(logger::LoggerHandler::Level::ERROR, "Invalid Content Selector.");
    return "";
}

std::string LocalMediaSourceHandler::convertPlayControlTypeToString(PlayControlType playControlType) {
    switch (playControlType) {
        case (PlayControlType::PAUSE):
            return "PAUSE";
        case (PlayControlType::RESUME):
            return "RESUME";
        case (PlayControlType::STOP):
            return "STOP";
        case (PlayControlType::NEXT):
            return "NEXT";
        case (PlayControlType::PREVIOUS):
            return "PREVIOUS";
        case (PlayControlType::START_OVER):
            return "START_OVER";
        case (PlayControlType::FAST_FORWARD):
            return "FAST_FORWARD";
        case (PlayControlType::REWIND):
            return "REWIND";
        case (PlayControlType::ENABLE_REPEAT_ONE):
            return "ENABLE_REPEAT_ONE";
        case (PlayControlType::ENABLE_REPEAT):
            return "ENABLE_REPEAT";
        case (PlayControlType::DISABLE_REPEAT):
            return "DISABLE_REPEAT";
        case (PlayControlType::ENABLE_SHUFFLE):
            return "ENABLE_SHUFFLE";
        case (PlayControlType::DISABLE_SHUFFLE):
            return "DISABLE_SHUFFLE";
        case (PlayControlType::FAVORITE):
            return "FAVORITE";
        case (PlayControlType::UNFAVORITE):
            return "UNFAVORITE";
    }
    // invalid play control type
    log(logger::LoggerHandler::Level::ERROR, "Invalid Play Control Type.");
    return "";
}

}  // namespace alexa
}  // namespace sampleApp
