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

#include "SampleApp/Alexa/LocalMediaSourceHandler.h"

// C++ Standard Library
#include <chrono>
#include <sstream>
#include <string>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocalMediaSourceHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LocalMediaSourceHandler::LocalMediaSourceHandler(std::weak_ptr<Activity> activity,
                                                 std::weak_ptr<logger::LoggerHandler> loggerHandler,
                                                 Source source,
                                                 std::shared_ptr<aace::alexa::Speaker> speaker)
    : aace::alexa::LocalMediaSource{source, speaker}, m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    // Expects(speaker != nullptr);
    setupUI();
}

std::weak_ptr<Activity> LocalMediaSourceHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> LocalMediaSourceHandler::getLoggerHandler() { return m_loggerHandler; }

// aace::alexa::Alerts interface

bool LocalMediaSourceHandler::authorize(bool authorized) {
    log(logger::LoggerHandler::Level::INFO, "authorize:authorized=" + std::string(authorized ? "true" : "false"));
    if (auto activity = m_activity.lock()) {
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printLine("Local CD Player authorize:", authorized ? "true" : "false");
            }
        });
    }
    return m_authorized = authorized;
}

bool LocalMediaSourceHandler::play(const std::string &payload) {
    log(logger::LoggerHandler::Level::INFO, "play:payload=" + payload);
    if (m_authorized) {
        if (auto activity = m_activity.lock()) {
            activity->runOnUIThread([=]() {
                setFocus();
                if (auto console = m_console.lock()) {
                    console->printLine("Local CD Player play:", payload);
                }
            });
        }
        return true;
    }
    return false;
}

bool LocalMediaSourceHandler::playControl(PlayControlType controlType) {
    std::stringstream ss;
    ss << controlType;
    log(logger::LoggerHandler::Level::INFO, "playControl:controlType=" + ss.str());
    if (m_authorized) {
        if (auto activity = m_activity.lock()) {
            auto text = ss.str();
            activity->runOnUIThread([=]() {
                if (auto console = m_console.lock()) {
                    console->printLine("Local CD Player play control:", text);
                }
            });
        }
        return true;
    }
    return false;
}

bool LocalMediaSourceHandler::seek(std::chrono::milliseconds offset) {
    log(logger::LoggerHandler::Level::INFO, "seek:offset=" + std::to_string(offset.count()));
    if (m_authorized) {
        if (auto activity = m_activity.lock()) {
            activity->runOnUIThread([=]() {
                if (auto console = m_console.lock()) {
                    console->printLine("Local CD Player seek:", offset.count());
                }
            });
        }
        return true;
    }
    return false;
}

bool LocalMediaSourceHandler::adjustSeek(std::chrono::milliseconds deltaOffset) {
    log(logger::LoggerHandler::Level::INFO, "adjustSeek:deltaOffset=" + std::to_string(deltaOffset.count()));
    if (m_authorized) {
        if (auto activity = m_activity.lock()) {
            activity->runOnUIThread([=]() {
                if (auto console = m_console.lock()) {
                    console->printLine("Local CD Player adjust seek:", deltaOffset.count());
                }
            });
        }
        return true;
    }
    return false;
}

LocalMediaSourceHandler::LocalMediaSourceState LocalMediaSourceHandler::getState() {
    auto state = LocalMediaSourceHandler::LocalMediaSourceState{};
    std::vector<aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation> supportedOperations{
        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PLAY, aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::PAUSE,
        aace::alexa::ExternalMediaAdapter::SupportedPlaybackOperation::STOP};
    state.playbackState.state = "IDLE";
    state.playbackState.supportedOperations = supportedOperations;
    state.playbackState.trackOffset = std::chrono::milliseconds(0);
    state.playbackState.shuffleEnabled = false;
    state.playbackState.repeatEnabled = false;
    state.playbackState.favorites = aace::alexa::ExternalMediaAdapter::Favorites::NOT_RATED;
    state.playbackState.type = "ExternalMediaPlayerMusicItem";
    state.playbackState.playbackSource = "mock playbackSource";
    state.playbackState.playbackSourceId = "mock playbackSourceId";
    state.playbackState.trackName = "mock trackName";
    state.playbackState.trackId = "";
    state.playbackState.trackNumber = "mock trackNumber";
    state.playbackState.artistName = "mock artistName";
    state.playbackState.artistId = "";
    state.playbackState.albumName = "mock albumName";
    state.playbackState.albumId = "";
    state.playbackState.mediaProvider = "Local CD Player";
    state.playbackState.mediaType = aace::alexa::ExternalMediaAdapter::MediaType::TRACK;
    state.playbackState.duration = std::chrono::milliseconds(60000);
    state.sessionState.endpointId = "localId";
    state.sessionState.loggedIn = false;
    state.sessionState.userName = "";
    state.sessionState.isGuest = false;
    state.sessionState.launched = false;
    state.sessionState.active = false;
    state.sessionState.accessToken = "";
    state.sessionState.tokenRefreshInterval = std::chrono::milliseconds(0);
    state.sessionState.playerCookie = "mock playerCookie";
    state.sessionState.spiVersion = "1.0";
    // if (auto console = m_console.lock()) {
    //     console->printLine("getState");
    // }
    return state;
}

// private

void LocalMediaSourceHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
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

} // namespace alexa
} // namespace sampleApp
