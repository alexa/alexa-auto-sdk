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

#include "SampleApp/DefaultMediaPlayer.h"

#include "SampleApp/ApplicationContext.h"

// C++ Standard Library
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DefaultMediaPlayer
//
////////////////////////////////////////////////////////////////////////////////////////////////////

DefaultMediaPlayer::DefaultMediaPlayer(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler, const std::string &identity)
    : m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)}, m_identity{identity} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> DefaultMediaPlayer::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> DefaultMediaPlayer::getLoggerHandler() { return m_loggerHandler; }

// aace::alexa::MediaPlayer interface

bool DefaultMediaPlayer::prepare() {
    log(logger::LoggerHandler::Level::INFO, "prepare");
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }
    m_tempPath = m_applicationContext->makeTempPath(m_identity, "mp3");
    if (std::remove(m_tempPath.c_str()) == 0) {
        log(logger::LoggerHandler::Level::INFO, "File successfully deleted: " + m_tempPath);
    }
    auto output = std::make_shared<std::ofstream>(m_tempPath, std::ios::binary | std::ofstream::out | std::ofstream::app);
    if (!output->good()) {
        log(logger::LoggerHandler::Level::INFO, "Could not create cache file: " + m_tempPath);
        return false;
    }
    log(logger::LoggerHandler::Level::VERBOSE, "Temporary file created: " + m_tempPath);
    char buffer[4096];
    ssize_t count = read(buffer, 4096);
    ssize_t bytes = 0;
    while (!isClosed()) {
        bytes += count;
        output->write(buffer, count);
        count = read(buffer, 4096);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    output->flush();
    output->close();
    return true;
}

bool DefaultMediaPlayer::prepare(const std::string &url) {
    std::stringstream ss;
    ss << "prepare with url is " << url;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    return false;
}

bool DefaultMediaPlayer::play() {
    log(logger::LoggerHandler::Level::INFO, "play");
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }
    auto executor = activity->getExecutor();
    if (!executor) {
        return false;
    }
    log(logger::LoggerHandler::Level::INFO, "MediaState::PLAYING");
    mediaStateChanged(MediaState::PLAYING);
    auto command = m_applicationContext->getMediaPlayerCommand();
    if (!command.empty()) {
        if (auto console = m_console.lock()) {
            console->print(m_applicationContext->executeCommand((command + ' ' + m_tempPath).c_str()));
        }
    }
    executor->submit([=]() {
        log(logger::LoggerHandler::Level::INFO, "MediaState::STOPPED");
        mediaStateChanged(MediaState::STOPPED);
    });
    return true;
}

bool DefaultMediaPlayer::stop() {
    log(logger::LoggerHandler::Level::INFO, "stop");
    return false;
}

bool DefaultMediaPlayer::pause() {
    log(logger::LoggerHandler::Level::INFO, "pause");
    return false;
}

bool DefaultMediaPlayer::resume() {
    log(logger::LoggerHandler::Level::INFO, "resume");
    return false;
}

int64_t DefaultMediaPlayer::getPosition() {
    log(logger::LoggerHandler::Level::INFO, "getPosition");
    return false;
}

bool DefaultMediaPlayer::setPosition(int64_t position) {
    std::stringstream ss;
    ss << "setPosition with position is " << std::to_string(position);
    log(logger::LoggerHandler::Level::INFO, ss.str());
    return false;
}

// aace::alexa::Speaker interface

bool DefaultMediaPlayer::setVolume(int8_t volume) { return true; }

bool DefaultMediaPlayer::adjustVolume(int8_t delta) { return true; }

bool DefaultMediaPlayer::setMute(bool mute) { return true; }

int8_t DefaultMediaPlayer::getVolume() { return m_applicationContext->getMaximumAVSVolume(); }

bool DefaultMediaPlayer::isMuted() { return false; }

// private

void DefaultMediaPlayer::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "DefaultMediaPlayer", message);
}

void DefaultMediaPlayer::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_applicationContext = activity->getApplicationContext();
    m_console = activity->findViewById("id:console");
}

} // namespace sampleApp
