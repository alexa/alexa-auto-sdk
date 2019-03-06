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
#include "MediaPlayerHandler.h"
#include "PlatformSpecificLoggingMacros.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace alexa {

const std::string TAG = "agl::alexa::MediaPlayerHandler";
static const std::chrono::seconds TIME_OUT_IN_SECS{5};

std::shared_ptr<MediaPlayerHandler> MediaPlayerHandler::create(
    const std::string& name,
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher,
    std::string media_file) {
    auto mediaPlayerHandler = std::shared_ptr<MediaPlayerHandler>(new MediaPlayerHandler());

    mediaPlayerHandler->m_topic = name;
    mediaPlayerHandler->m_logger = logger;
    mediaPlayerHandler->m_directiveDispatcher = directiveDispatcher;
    mediaPlayerHandler->m_media_file = media_file;
    mediaPlayerHandler->m_mediaplayer_position_set = false;
    return mediaPlayerHandler;
}

bool MediaPlayerHandler::prepare() {
    m_logger->log(Level::VERBOSE, TAG, "Prepare " + m_topic);
    const size_t size = 4096;
    char buffer[size];
    std::ofstream file;
    file.open(m_media_file, std::ofstream::trunc);
    while (!(isClosed())) {
        ssize_t readCount = read((char*)buffer, size);
        file.write((char*)&buffer, readCount);
    }
    file.close();

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(m_topic, __FUNCTION__, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }

    return true;
}

bool MediaPlayerHandler::prepare(const std::string& url) {
    m_logger->log(Level::VERBOSE, TAG, "Prepare " + m_topic);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(m_topic, __FUNCTION__, url);
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }

    return true;
}

bool MediaPlayerHandler::play() {
    m_logger->log(Level::INFO, TAG, "play " + m_topic);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(m_topic, __FUNCTION__, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }

    return true;
}

bool MediaPlayerHandler::stop() {
    m_logger->log(Level::INFO, TAG, "stop " + m_topic);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(m_topic, __FUNCTION__, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }

    return true;
}

bool MediaPlayerHandler::pause() {
    m_logger->log(Level::VERBOSE, TAG, "pause " + m_topic);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(m_topic, __FUNCTION__, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }

    return true;
}

bool MediaPlayerHandler::resume() {
    m_logger->log(Level::VERBOSE, TAG, "resume " + m_topic);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(m_topic, __FUNCTION__, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }

    return true;
}

int64_t MediaPlayerHandler::getPosition() {
    m_logger->log(Level::VERBOSE, TAG, "getPosition " + m_topic);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(m_topic, __FUNCTION__, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    // Block until we receive MediaPlayer position
    if (m_cv_position.wait_for(lock, TIME_OUT_IN_SECS, [this]() { return m_mediaplayer_position_set == true; })) {
        m_mediaplayer_position_set = false;
        return m_mediaplayer_position;
    }

    AASB_WARNING("Timed out due to not receiving mediaplayer position.");
    return -1;
}

bool MediaPlayerHandler::setPosition(int64_t position) {
    m_logger->log(Level::VERBOSE, TAG, "setPosition " + m_topic);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(m_topic, __FUNCTION__, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }

    return true;
}

void MediaPlayerHandler::onMediaPlayerPositionReceived(int64_t position) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_mediaplayer_position = position;
        m_mediaplayer_position_set = true;
    }
    m_cv_position.notify_one();
}

void MediaPlayerHandler::onMediaStateChangedEvent(const std::string& mediaStateStr) {
    MediaState mediaState = toMediaState(mediaStateStr);
    mediaStateChanged(mediaState);
}

void MediaPlayerHandler::onMediaErrorEvent(const std::string& mediaErrorStr, const std::string& description) {
    MediaError mediaError = toMediaError(mediaErrorStr);
    MediaPlayerHandler::mediaError(mediaError, description);
}

MediaPlayerHandler::MediaState MediaPlayerHandler::toMediaState(const std::string& mediaStateStr) {
    if (mediaStateStr == "PLAYING") return MediaState::PLAYING;
    if (mediaStateStr == "BUFFERING") return MediaState::BUFFERING;
    return MediaState::STOPPED;
}

MediaPlayerHandler::MediaError MediaPlayerHandler::toMediaError(const std::string& mediaErrorStr) {
    if (mediaErrorStr == "MEDIA_ERROR_INVALID_REQUEST") return MediaError::MEDIA_ERROR_INVALID_REQUEST;
    if (mediaErrorStr == "MEDIA_ERROR_SERVICE_UNAVAILABLE") return MediaError::MEDIA_ERROR_SERVICE_UNAVAILABLE;
    if (mediaErrorStr == "MEDIA_ERROR_INTERNAL_SERVER_ERROR") return MediaError::MEDIA_ERROR_INTERNAL_SERVER_ERROR;
    if (mediaErrorStr == "MEDIA_ERROR_INTERNAL_DEVICE_ERROR") return MediaError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR;
    return MediaError::MEDIA_ERROR_UNKNOWN;
}

}  // namespace alexa
}  // namespace aasb