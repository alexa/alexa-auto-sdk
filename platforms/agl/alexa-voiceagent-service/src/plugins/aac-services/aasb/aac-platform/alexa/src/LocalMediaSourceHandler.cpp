/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "LocalMediaSourceHandler.h"

#include <aasb/Consts.h>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace alexa {

using namespace aasb::bridge;

using namespace rapidjson;

const std::string TAG = "aasb::alexa::LocalMediaSourceHandler";

static const std::chrono::seconds TIME_OUT_IN_SECS{5};

std::shared_ptr<LocalMediaSourceHandler> LocalMediaSourceHandler::create(
    Source source,
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    auto localMediaSourceHandler =
        std::shared_ptr<LocalMediaSourceHandler>(new LocalMediaSourceHandler(source, responseDispatcher));

    localMediaSourceHandler->m_logger = logger;

    return localMediaSourceHandler;
}

LocalMediaSourceHandler::LocalMediaSourceHandler(
    Source source,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) :
        aace::alexa::LocalMediaSource(source),
        m_responseDispatcher(responseDispatcher) {
    m_SourceString = convertSourceToString(source);
}

bool LocalMediaSourceHandler::play(ContentSelector contentSelectorType, const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "play " + TOPIC_LOCAL_MEDIA_SOURCE);

    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        rapidjson::Document document;
        document.SetObject();
        document.AddMember(
            "source",
            rapidjson::Value().SetString(m_SourceString.c_str(), m_SourceString.length()),
            document.GetAllocator());
        std::string contentSelectorTypeString = convertContentSelectorTypeToString(contentSelectorType);
        document.AddMember(
            "contentSelectorType",
            rapidjson::Value().SetString(contentSelectorTypeString.c_str(), contentSelectorTypeString.length()),
            document.GetAllocator());
        document.AddMember(
            "payload", rapidjson::Value().SetString(payload.c_str(), payload.length()), document.GetAllocator());

        // create event string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);

        responseDispatcher->sendDirective(TOPIC_LOCAL_MEDIA_SOURCE, ACTION_LOCAL_MEDIA_SOURCE_PLAY, buffer.GetString());
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    return true;
}

bool LocalMediaSourceHandler::playControl(PlayControlType controlType) {
    m_logger->log(Level::VERBOSE, TAG, "playControl " + TOPIC_LOCAL_MEDIA_SOURCE);

    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        rapidjson::Document document;
        document.SetObject();
        document.AddMember(
            "source",
            rapidjson::Value().SetString(m_SourceString.c_str(), m_SourceString.length()),
            document.GetAllocator());

        rapidjson::Value payloadElement;

        payloadElement.SetObject();
        std::string controlTypeString = convertPlayControlTypeToString(controlType);
        payloadElement.AddMember(
            "playControlType",
            rapidjson::Value().SetString(controlTypeString.c_str(), controlTypeString.length()),
            document.GetAllocator());

        document.AddMember("payload", payloadElement, document.GetAllocator());

        // create event string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);

        responseDispatcher->sendDirective(
            TOPIC_LOCAL_MEDIA_SOURCE, ACTION_LOCAL_MEDIA_SOURCE_PLAY_CONTROL, buffer.GetString());
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    return true;
}

bool LocalMediaSourceHandler::seek(std::chrono::milliseconds offset) {
    m_logger->log(Level::VERBOSE, TAG, "seek " + TOPIC_LOCAL_MEDIA_SOURCE);

    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        rapidjson::Document document;
        document.SetObject();
        document.AddMember(
            "source",
            rapidjson::Value().SetString(m_SourceString.c_str(), m_SourceString.length()),
            document.GetAllocator());

        rapidjson::Value payloadElement;

        payloadElement.SetObject();

        int duration = offset.count();
        payloadElement.AddMember("duration", rapidjson::Value().SetInt(duration), document.GetAllocator());

        document.AddMember("payload", payloadElement, document.GetAllocator());

        // create event string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);

        responseDispatcher->sendDirective(TOPIC_LOCAL_MEDIA_SOURCE, ACTION_LOCAL_MEDIA_SOURCE_SEEK, buffer.GetString());
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    return true;
}

bool LocalMediaSourceHandler::adjustSeek(std::chrono::milliseconds deltaOffset) {
    m_logger->log(Level::VERBOSE, TAG, "adjustSeek " + TOPIC_LOCAL_MEDIA_SOURCE);

    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        rapidjson::Document document;
        document.SetObject();
        document.AddMember(
            "source",
            rapidjson::Value().SetString(m_SourceString.c_str(), m_SourceString.length()),
            document.GetAllocator());

        rapidjson::Value payloadElement;

        payloadElement.SetObject();

        int duration = deltaOffset.count();
        payloadElement.AddMember("duration", rapidjson::Value().SetInt(duration), document.GetAllocator());

        document.AddMember("payload", payloadElement, document.GetAllocator());

        // create event string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);

        responseDispatcher->sendDirective(
            TOPIC_LOCAL_MEDIA_SOURCE, ACTION_LOCAL_MEDIA_SOURCE_ADJUST_SEEK, buffer.GetString());
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    return true;
}

LocalMediaSourceHandler::LocalMediaSourceState LocalMediaSourceHandler::getState() {
    m_logger->log(Level::VERBOSE, TAG, "getState " + TOPIC_LOCAL_MEDIA_SOURCE);
    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        rapidjson::Document document;
        document.SetObject();
        document.AddMember(
            "source",
            rapidjson::Value().SetString(m_SourceString.c_str(), m_SourceString.length()),
            document.GetAllocator());

        // create event string
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);

        responseDispatcher->sendDirective(
            TOPIC_LOCAL_MEDIA_SOURCE, ACTION_LOCAL_MEDIA_SOURCE_GET_STATE, buffer.GetString());

        std::unique_lock<std::mutex> lock(m_mutex);
        // Block until we receive MediaPlayer position
        if (m_getStateResponseCv.wait_for(lock, TIME_OUT_IN_SECS, [this]() { return m_didReceiveGetStateResponse; })) {
            m_didReceiveGetStateResponse = false;
            return m_mediaSourceState;
        }

        m_logger->log(Level::WARN, TAG, "getState request timed out");
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
    }

    return LocalMediaSourceHandler::LocalMediaSourceState{};
}

void LocalMediaSourceHandler::onReceivedEvent(const std::string& action, const std::string& payload) {
    m_logger->log(Level::INFO, TAG, "onReceivedEvent: " + action);

    if (action == aasb::bridge::ACTION_LOCAL_MEDIA_SOURCE_GET_STATE_RESPONSE) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_didReceiveGetStateResponse = setMediaSourceState(payload);
        }
        m_getStateResponseCv.notify_one();
    } else if (action == aasb::bridge::ACTION_LOCAL_MEDIA_SOURCE_PLAYER_EVENT) {
        rapidjson::Document document;
        document.Parse(payload.c_str());
        auto root = document.GetObject();

        if (root.HasMember(JSON_ATTR_PLAYER_EVENT_NAME.c_str()) &&
            root[JSON_ATTR_PLAYER_EVENT_NAME.c_str()].IsString()) {
            auto eventName = root[JSON_ATTR_PLAYER_EVENT_NAME.c_str()].GetString();
            playerEvent(eventName);
        } else {
            m_logger->log(Level::ERROR, TAG, "LocalMediaSourceHandler: no player event.");
        }
    } else if (action == aasb::bridge::ACTION_LOCAL_MEDIA_SOURCE_PLAYER_ERROR) {
        rapidjson::Document document;
        document.Parse(payload.c_str());
        auto root = document.GetObject();

        std::string errorName;
        long code;
        std::string description;
        bool fatal;
        if (root.HasMember(JSON_ATTR_PLAYER_ERROR_NAME.c_str()) &&
            root[JSON_ATTR_PLAYER_ERROR_NAME.c_str()].IsString()) {
            errorName = root[JSON_ATTR_PLAYER_ERROR_NAME.c_str()].GetString();
        } else {
            m_logger->log(Level::ERROR, TAG, "LocalMediaSourceHandler: no error name.");
            return;
        }

        if (root.HasMember(JSON_ATTR_PLAYER_ERROR_CODE.c_str()) && root[JSON_ATTR_PLAYER_ERROR_CODE.c_str()].IsInt()) {
            code = root[JSON_ATTR_PLAYER_ERROR_CODE.c_str()].GetInt();
        } else {
            m_logger->log(Level::ERROR, TAG, "LocalMediaSourceHandler: no error code.");
            return;
        }

        if (root.HasMember(JSON_ATTR_PLAYER_DESCRIPTION.c_str()) &&
            root[JSON_ATTR_PLAYER_DESCRIPTION.c_str()].IsString()) {
            description = root[JSON_ATTR_PLAYER_DESCRIPTION.c_str()].GetString();
        } else {
            m_logger->log(Level::ERROR, TAG, "LocalMediaSourceHandler: no description.");
            return;
        }

        if (root.HasMember(JSON_ATTR_PLAYER_FATAL.c_str()) && root[JSON_ATTR_PLAYER_FATAL.c_str()].IsBool()) {
            fatal = root[JSON_ATTR_PLAYER_FATAL.c_str()].GetBool();
        } else {
            m_logger->log(Level::ERROR, TAG, "LocalMediaSourceHandler: no fatal.");
            return;
        }

        playerError(errorName, code, description, fatal);
    } else {
        m_logger->log(Level::ERROR, TAG, "LocalMediaSourceHandler: action unknown.");
    }
}

bool LocalMediaSourceHandler::setMediaSourceState(const std::string& payload) {
    rapidjson::Document document;
    document.Parse(payload.c_str());
    auto root = document.GetObject();

    if (root.HasMember(JSON_ATTR_SESSION_STATE.c_str()) && root[JSON_ATTR_SESSION_STATE.c_str()].IsObject()) {
        rapidjson::Value sessionStateElement = root[JSON_ATTR_SESSION_STATE.c_str()].GetObject();

        if (sessionStateElement.HasMember(JSON_ATTR_END_POINT_ID.c_str()) &&
            sessionStateElement[JSON_ATTR_END_POINT_ID.c_str()].IsString()) {
            m_mediaSourceState.sessionState.endpointId =
                sessionStateElement[JSON_ATTR_END_POINT_ID.c_str()].GetString();
        } else {
            m_logger->log(Level::WARN, TAG, "endpointId doesn't exist.");
        }

        if (sessionStateElement.HasMember(JSON_ATTR_LOGGED_IN.c_str()) &&
            sessionStateElement[JSON_ATTR_LOGGED_IN.c_str()].IsBool()) {
            m_mediaSourceState.sessionState.loggedIn = sessionStateElement[JSON_ATTR_LOGGED_IN.c_str()].GetBool();
        } else {
            m_logger->log(Level::WARN, TAG, "loggedIn doesn't exist.");
        }

        if (sessionStateElement.HasMember(JSON_ATTR_USER_NAME.c_str()) &&
            sessionStateElement[JSON_ATTR_USER_NAME.c_str()].IsString()) {
            m_mediaSourceState.sessionState.userName = sessionStateElement[JSON_ATTR_USER_NAME.c_str()].GetString();
        } else {
            m_logger->log(Level::WARN, TAG, "userName doesn't exist.");
        }

        if (sessionStateElement.HasMember(JSON_ATTR_IS_GUEST.c_str()) &&
            sessionStateElement[JSON_ATTR_IS_GUEST.c_str()].IsBool()) {
            m_mediaSourceState.sessionState.isGuest = sessionStateElement[JSON_ATTR_IS_GUEST.c_str()].GetBool();
        } else {
            m_logger->log(Level::WARN, TAG, "isGuest doesn't exist.");
        }

        if (sessionStateElement.HasMember(JSON_ATTR_LAUNCHED.c_str()) &&
            sessionStateElement[JSON_ATTR_LAUNCHED.c_str()].IsBool()) {
            m_mediaSourceState.sessionState.launched = sessionStateElement[JSON_ATTR_LAUNCHED.c_str()].GetBool();
        } else {
            m_logger->log(Level::WARN, TAG, "launched doesn't exist.");
        }

        if (sessionStateElement.HasMember(JSON_ATTR_ACTIVE.c_str()) &&
            sessionStateElement[JSON_ATTR_ACTIVE.c_str()].IsBool()) {
            m_mediaSourceState.sessionState.active = sessionStateElement[JSON_ATTR_ACTIVE.c_str()].GetBool();
        } else {
            m_logger->log(Level::WARN, TAG, "active doesn't exist.");
        }

        if (sessionStateElement.HasMember(JSON_ATTR_ACCESS_TOKEN.c_str()) &&
            sessionStateElement[JSON_ATTR_ACCESS_TOKEN.c_str()].IsString()) {
            m_mediaSourceState.sessionState.accessToken =
                sessionStateElement[JSON_ATTR_ACCESS_TOKEN.c_str()].GetString();
        } else {
            m_logger->log(Level::WARN, TAG, "accessToken doesn't exist.");
        }

        if (sessionStateElement.HasMember(JSON_ATTR_TOKEN_REFRESH_INTERVAL.c_str()) &&
            sessionStateElement[JSON_ATTR_TOKEN_REFRESH_INTERVAL.c_str()].IsInt()) {
            auto interval = sessionStateElement[JSON_ATTR_TOKEN_REFRESH_INTERVAL.c_str()].GetInt();
            std::chrono::milliseconds intervalms(interval);
            m_mediaSourceState.sessionState.tokenRefreshInterval = intervalms;
        } else {
            m_logger->log(Level::WARN, TAG, "tokenRefreshInterval doesn't exist.");
        }

        if (sessionStateElement.HasMember(JSON_ATTR_SUPPORTED_CONTENT_SELECTORS.c_str()) &&
            sessionStateElement[JSON_ATTR_SUPPORTED_CONTENT_SELECTORS.c_str()].IsArray()) {
            rapidjson::Value supportedContentSelectorsElement =
                sessionStateElement[JSON_ATTR_SUPPORTED_CONTENT_SELECTORS.c_str()].GetArray();
            for (unsigned int j = 0; j < supportedContentSelectorsElement.Size(); j++) {
                auto next = supportedContentSelectorsElement[j].GetString();
                if (VALUE_FREQUENCY.compare(next) == 0) {
                    m_mediaSourceState.sessionState.supportedContentSelectors.push_back(
                        aace::alexa::LocalMediaSource::ContentSelector::FREQUENCY);
                } else if (VALUE_CHANNEL.compare(next) == 0) {
                    m_mediaSourceState.sessionState.supportedContentSelectors.push_back(
                        aace::alexa::LocalMediaSource::ContentSelector::CHANNEL);
                } else if (VALUE_PRESET.compare(next) == 0) {
                    m_mediaSourceState.sessionState.supportedContentSelectors.push_back(
                        aace::alexa::LocalMediaSource::ContentSelector::PRESET);
                } else {
                    m_logger->log(Level::WARN, TAG, "setMediaSourceState: Unknown supported content selector " + payload);
                }
            }
        } else {
            m_logger->log(Level::WARN, TAG, "supportedContentSelectors doesn't exist.");
        }

        if (sessionStateElement.HasMember(JSON_ATTR_SPI_VERSION.c_str()) &&
            sessionStateElement[JSON_ATTR_SPI_VERSION.c_str()].IsString()) {
            m_mediaSourceState.sessionState.spiVersion = sessionStateElement[JSON_ATTR_SPI_VERSION.c_str()].GetString();
        } else {
            m_logger->log(Level::WARN, TAG, "spiVersion doesn't exist.");
        }
    } else {
        m_mediaSourceState = LocalMediaSourceHandler::LocalMediaSourceState{};
        m_logger->log(Level::ERROR, TAG, "setMediaSourceState: No sessionState " + payload);
        return false;
    }

    if (root.HasMember(JSON_ATTR_PLAYBACK_STATE.c_str()) && root[JSON_ATTR_PLAYBACK_STATE.c_str()].IsObject()) {
        rapidjson::Value playbackStateElement = root[JSON_ATTR_PLAYBACK_STATE.c_str()].GetObject();
        if (playbackStateElement.HasMember(JSON_ATTR_STATE.c_str()) &&
            playbackStateElement[JSON_ATTR_STATE.c_str()].IsString()) {
            m_mediaSourceState.playbackState.state = playbackStateElement[JSON_ATTR_STATE.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_SUPPORTED_OPERATIONS.c_str()) &&
            playbackStateElement[JSON_ATTR_SUPPORTED_OPERATIONS.c_str()].IsArray()) {
            rapidjson::Value supportedOperationsElement =
                playbackStateElement[JSON_ATTR_SUPPORTED_OPERATIONS.c_str()].GetArray();
            for (unsigned int j = 0; j < supportedOperationsElement.Size(); j++) {
                auto next = supportedOperationsElement[j].GetString();
                if (VALUE_PLAY.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PLAY);
                } else if (VALUE_RESUME.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::RESUME);
                } else if (VALUE_PAUSE.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PAUSE);
                } else if (VALUE_RESUME.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::RESUME);
                } else if (VALUE_STOP.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::STOP);
                } else if (VALUE_NEXT.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::NEXT);
                } else if (VALUE_PREVIOUS.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PREVIOUS);
                } else if (VALUE_START_OVER.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::START_OVER);
                } else if (VALUE_FAST_FORWARD.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::FAST_FORWARD);
                } else if (VALUE_REWIND.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::REWIND);
                } else if (VALUE_ENABLE_REPEAT.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT);
                } else if (VALUE_ENABLE_REPEAT_ONE.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_REPEAT_ONE);
                } else if (VALUE_DISABLE_REPEAT.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::DISABLE_REPEAT);
                } else if (VALUE_ENABLE_SHUFFLE.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ENABLE_SHUFFLE);
                } else if (VALUE_FAVORITE.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::FAVORITE);
                } else if (VALUE_UNFAVORITE.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::UNFAVORITE);
                } else if (VALUE_SEEK.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::SEEK);
                } else if (VALUE_ADJUST_SEEK.compare(next) == 0) {
                    m_mediaSourceState.playbackState.supportedOperations.push_back(
                        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::ADJUST_SEEK);
                } else {
                    m_logger->log(Level::WARN, TAG, "setMediaSourceState: Unknown supportedOperations " + payload);
                }
            }
        } else {
            m_logger->log(Level::WARN, TAG, "supportedOperations doesn't exist.");
        }

        if (playbackStateElement.HasMember(JSON_ATTR_TRACK_OFFSET.c_str()) &&
            playbackStateElement[JSON_ATTR_TRACK_OFFSET.c_str()].IsInt()) {
            auto count = playbackStateElement[JSON_ATTR_TRACK_OFFSET.c_str()].GetInt();
            std::chrono::milliseconds trackOffsetMs(count);
            m_mediaSourceState.playbackState.trackOffset = trackOffsetMs;
        }

        if (playbackStateElement.HasMember(JSON_ATTR_SHUFFLE_ENABLED.c_str()) &&
            playbackStateElement[JSON_ATTR_SHUFFLE_ENABLED.c_str()].IsBool()) {
            m_mediaSourceState.playbackState.shuffleEnabled =
                playbackStateElement[JSON_ATTR_SHUFFLE_ENABLED.c_str()].GetBool();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_REPEAT_ENABLED.c_str()) &&
            playbackStateElement[JSON_ATTR_REPEAT_ENABLED.c_str()].IsBool()) {
            m_mediaSourceState.playbackState.repeatEnabled =
                playbackStateElement[JSON_ATTR_REPEAT_ENABLED.c_str()].GetBool();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_FAVORITES.c_str()) &&
            playbackStateElement[JSON_ATTR_FAVORITES.c_str()].IsBool()) {
            auto favoritesString = playbackStateElement[JSON_ATTR_FAVORITES.c_str()].GetString();
            if (VALUE_FAVORITED.compare(favoritesString) == 0) {
                m_mediaSourceState.playbackState.favorites = aace::alexa::ExternalMediaAdapter::Favorites::FAVORITED;
            } else if (VALUE_UNFAVORITED.compare(favoritesString) == 0) {
                m_mediaSourceState.playbackState.favorites = aace::alexa::ExternalMediaAdapter::Favorites::UNFAVORITED;
            } else if (VALUE_NOT_RATED.compare(favoritesString) == 0) {
                m_mediaSourceState.playbackState.favorites = aace::alexa::ExternalMediaAdapter::Favorites::NOT_RATED;
            } else {
                m_logger->log(Level::WARN, TAG, "setMediaSourceState: Unknown favorites " + payload);
            }
        }

        if (playbackStateElement.HasMember(JSON_ATTR_TYPE.c_str()) &&
            playbackStateElement[JSON_ATTR_TYPE.c_str()].IsString()) {
            m_mediaSourceState.playbackState.type = playbackStateElement[JSON_ATTR_TYPE.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_SOURCE.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_SOURCE.c_str()].IsString()) {
            m_mediaSourceState.playbackState.playbackSource =
                playbackStateElement[JSON_ATTR_PLAYBACK_SOURCE.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_SOURCE_ID.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_SOURCE_ID.c_str()].IsString()) {
            m_mediaSourceState.playbackState.playbackSourceId =
                playbackStateElement[JSON_ATTR_PLAYBACK_SOURCE_ID.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_TRACK_NAME.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_TRACK_NAME.c_str()].IsString()) {
            m_mediaSourceState.playbackState.trackName =
                playbackStateElement[JSON_ATTR_PLAYBACK_TRACK_NAME.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_TRACK_ID.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_TRACK_ID.c_str()].IsString()) {
            m_mediaSourceState.playbackState.trackId =
                playbackStateElement[JSON_ATTR_PLAYBACK_TRACK_ID.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_TRACK_NUMBER.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_TRACK_NUMBER.c_str()].IsString()) {
            m_mediaSourceState.playbackState.trackNumber =
                playbackStateElement[JSON_ATTR_PLAYBACK_TRACK_NUMBER.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_ARTIST_NAME.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_ARTIST_NAME.c_str()].IsString()) {
            m_mediaSourceState.playbackState.artistName =
                playbackStateElement[JSON_ATTR_PLAYBACK_ARTIST_NAME.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_ARTIST_ID.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_ARTIST_ID.c_str()].IsString()) {
            m_mediaSourceState.playbackState.artistId =
                playbackStateElement[JSON_ATTR_PLAYBACK_ARTIST_ID.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_ALBUM_NAME.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_ALBUM_NAME.c_str()].IsString()) {
            m_mediaSourceState.playbackState.albumName =
                playbackStateElement[JSON_ATTR_PLAYBACK_ALBUM_NAME.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_ALBUM_ID.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_ALBUM_ID.c_str()].IsString()) {
            m_mediaSourceState.playbackState.albumId =
                playbackStateElement[JSON_ATTR_PLAYBACK_ALBUM_ID.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_TINY_URL.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_TINY_URL.c_str()].IsString()) {
            m_mediaSourceState.playbackState.tinyURL =
                playbackStateElement[JSON_ATTR_PLAYBACK_TINY_URL.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_SMALL_URL.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_SMALL_URL.c_str()].IsString()) {
            m_mediaSourceState.playbackState.smallURL =
                playbackStateElement[JSON_ATTR_PLAYBACK_SMALL_URL.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_MEDIUM_URL.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_MEDIUM_URL.c_str()].IsString()) {
            m_mediaSourceState.playbackState.mediumURL =
                playbackStateElement[JSON_ATTR_PLAYBACK_MEDIUM_URL.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_LARGE_URL.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_LARGE_URL.c_str()].IsString()) {
            m_mediaSourceState.playbackState.largeURL =
                playbackStateElement[JSON_ATTR_PLAYBACK_LARGE_URL.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_COVER_ID.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_COVER_ID.c_str()].IsString()) {
            m_mediaSourceState.playbackState.coverId =
                playbackStateElement[JSON_ATTR_PLAYBACK_COVER_ID.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_PLAYBACK_MEDIA_PROVIDER.c_str()) &&
            playbackStateElement[JSON_ATTR_PLAYBACK_MEDIA_PROVIDER.c_str()].IsString()) {
            m_mediaSourceState.playbackState.mediaProvider =
                playbackStateElement[JSON_ATTR_PLAYBACK_MEDIA_PROVIDER.c_str()].GetString();
        }

        if (playbackStateElement.HasMember(JSON_ATTR_MEDIA_TYPE.c_str()) &&
            playbackStateElement[JSON_ATTR_MEDIA_TYPE.c_str()].IsBool()) {
            auto mediaTypeString = playbackStateElement[JSON_ATTR_MEDIA_TYPE.c_str()].GetString();
            if (VALUE_TRACK.compare(mediaTypeString) == 0) {
                m_mediaSourceState.playbackState.mediaType = aace::alexa::ExternalMediaAdapter::MediaType::TRACK;
            } else if (VALUE_PODCAST.compare(mediaTypeString) == 0) {
                m_mediaSourceState.playbackState.mediaType = aace::alexa::ExternalMediaAdapter::MediaType::PODCAST;
            } else if (VALUE_STATION.compare(mediaTypeString) == 0) {
                m_mediaSourceState.playbackState.mediaType = aace::alexa::ExternalMediaAdapter::MediaType::STATION;
            } else if (VALUE_AD.compare(mediaTypeString) == 0) {
                m_mediaSourceState.playbackState.mediaType = aace::alexa::ExternalMediaAdapter::MediaType::AD;
            } else if (VALUE_SAMPLE.compare(mediaTypeString) == 0) {
                m_mediaSourceState.playbackState.mediaType = aace::alexa::ExternalMediaAdapter::MediaType::SAMPLE;
            } else if (VALUE_OTHER.compare(mediaTypeString) == 0) {
                m_mediaSourceState.playbackState.mediaType = aace::alexa::ExternalMediaAdapter::MediaType::OTHER;
            } else {
                m_logger->log(Level::WARN, TAG, "setMediaSourceState: Unknown mediaType " + payload);
            }
        }

        if (playbackStateElement.HasMember(JSON_ATTR_DURATION.c_str()) &&
            playbackStateElement[JSON_ATTR_DURATION.c_str()].IsInt()) {
            auto count = playbackStateElement[JSON_ATTR_DURATION.c_str()].GetInt();
            std::chrono::milliseconds durationMs(count);
            m_mediaSourceState.playbackState.duration = durationMs;
        }
    } else {
        m_mediaSourceState = LocalMediaSourceHandler::LocalMediaSourceState{};
        m_logger->log(Level::WARN, TAG, "setMediaSourceState: No playbackState " + payload);
        return false;
    }

    return true;
}

bool LocalMediaSourceHandler::volumeChanged(float volume) {
    return true;
}

bool LocalMediaSourceHandler::mutedStateChanged(MutedState state) {
    return true;
}

std::string LocalMediaSourceHandler::convertPlayControlTypeToString(PlayControlType controlType) {
    switch (controlType) {
        case PlayControlType::PAUSE:
            return "PAUSE";
        case PlayControlType::RESUME:
            return "RESUME";
        case PlayControlType::NEXT:
            return "NEXT";
        case PlayControlType::PREVIOUS:
            return "PREVIOUS";
        case PlayControlType::START_OVER:
            return "START_OVER";
        case PlayControlType::FAST_FORWARD:
            return "FAST_FORWARD";
        case PlayControlType::REWIND:
            return "REWIND";
        case PlayControlType::ENABLE_REPEAT_ONE:
            return "ENABLE_REPEAT_ONE";
        case PlayControlType::DISABLE_REPEAT:
            return "DISABLE_REPEAT";
        case PlayControlType::ENABLE_SHUFFLE:
            return "ENABLE_SHUFFLE";
        case PlayControlType::DISABLE_SHUFFLE:
            return "DISABLE_SHUFFLE";
        case PlayControlType::FAVORITE:
            return "FAVORITE";
        case PlayControlType::UNFAVORITE:
            return "UNFAVORITE";
        default:
            return std::string("UNKNOWN");
    }
}

std::string LocalMediaSourceHandler::convertSourceToString(Source source) {
    switch (source) {
        case Source::BLUETOOTH:
            return VALUE_LOCAL_MEDIA_SOURCE_BLUETOOTH;
        case Source::USB:
            return VALUE_LOCAL_MEDIA_SOURCE_USB;
        case Source::FM_RADIO:
            return VALUE_LOCAL_MEDIA_SOURCE_FM_RADIO;
        case Source::AM_RADIO:
            return VALUE_LOCAL_MEDIA_SOURCE_AM_RADIO;
        case Source::SATELLITE_RADIO:
            return VALUE_LOCAL_MEDIA_SOURCE_SATELLITE_RADIO;
        case Source::LINE_IN:
            return VALUE_LOCAL_MEDIA_SOURCE_LINE_IN;
        case Source::COMPACT_DISC:
            return VALUE_LOCAL_MEDIA_SOURCE_COMPACT_DISC;
        default:
            return std::string("UNKNOWN");
    }
}

std::string LocalMediaSourceHandler::convertContentSelectorTypeToString(ContentSelector contentSelectorType) {
    switch (contentSelectorType) {
        case ContentSelector::FREQUENCY:
            return "FREQUENCY";
        case ContentSelector::CHANNEL:
            return "CHANNEL";
        case ContentSelector::PRESET:
            return "PRESET";
        default:
            return std::string("UNKNOWN");
    }
}

}  // namespace alexa
}  // namespace aasb