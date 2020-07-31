/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Audio/AudioOutputProviderHandler.h"

// C++ Standard Library
#include <sstream>
#include <array>
#include <random>
#include <fstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace audio {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioOutputProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AudioOutputProviderHandler::AudioOutputProviderHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    bool setup) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    // Expects((mediaPlayer != nullptr) && (speaker != nullptr));
    if (setup) {
        setupUI();
    }
}

std::weak_ptr<Activity> AudioOutputProviderHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AudioOutputProviderHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::audio::AudioOutputProvider interface

std::shared_ptr<aace::audio::AudioOutput> AudioOutputProviderHandler::openChannel(
    const std::string& name,
    AudioOutputType type) {
    auto activity = m_activity.lock();
    if (!activity) {
        return nullptr;
    }

    if (type == AudioOutputType::TTS) {
        return DefaultAudioOutput::create(
            activity->getApplicationContext(),
            m_loggerHandler,
            name,
            std::chrono::seconds(1),
            std::chrono::seconds(10),
            true);
    } else if (type == AudioOutputType::MUSIC) {
        return DefaultAudioOutput::create(
            activity->getApplicationContext(),
            m_loggerHandler,
            name,
            std::chrono::seconds(30),
            std::chrono::seconds(30));
    } else {
        return DefaultAudioOutput::create(activity->getApplicationContext(), m_loggerHandler, name);
    }
}

// private

void AudioOutputProviderHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AudioOutputProviderHandler", message);
}

void AudioOutputProviderHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DefaultAudioOutput
//
////////////////////////////////////////////////////////////////////////////////////////////////////

DefaultAudioOutput::DefaultAudioOutput(
    std::weak_ptr<ApplicationContext> applicationContext,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    const std::string& name,
    std::chrono::milliseconds minPlayDuration,
    std::chrono::milliseconds maxPlayDuration,
    bool enableOutput) :
        m_applicationContext{std::move(applicationContext)},
        m_loggerHandler{std::move(loggerHandler)},
        m_name{name},
        m_minPlayDuration{minPlayDuration},
        m_maxPlayDuration{maxPlayDuration},
        m_enableOutput{enableOutput},
        m_position{0} {
}

bool DefaultAudioOutput::prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) {
    log(logger::LoggerHandler::Level::INFO, "setStreamsetStream");
    if (m_enableOutput) {
        writeStreamToFile(stream);
    }
    return true;
}

bool DefaultAudioOutput::prepare(const std::string& url, bool repeating) {
    std::stringstream ss;
    ss << "prepare: " << url << " repeating: " << repeating;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    return true;
}

bool DefaultAudioOutput::play() {
    log(logger::LoggerHandler::Level::INFO, "play");
    m_playing = true;
    m_paused = false;
    mediaStateChanged(MediaState::PLAYING);
    m_executer.submit([=]() {
        std::random_device seeder;
        std::mt19937 engine(seeder());
        std::uniform_int_distribution<int> dist(m_minPlayDuration.count(), m_maxPlayDuration.count());
        int sleepTime = 100;
        int reportInterval = 1000;
        auto playTime = dist(engine);
        auto lastReportPosition = 0;
        while (m_position < playTime && m_playing) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [=] { return !m_paused; });
            m_position += sleepTime;
            if (lastReportPosition == 0 || (m_position - lastReportPosition) > reportInterval) {
                std::stringstream ss;

                ss << "Position: "
                   << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(m_position)).count()
                   << "s Remaining: "
                   << std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::milliseconds((playTime - m_position)))
                          .count()
                   << "s";

                log(logger::LoggerHandler::Level::VERBOSE, ss.str());
                lastReportPosition = m_position;
            }
        }
        log(logger::LoggerHandler::Level::INFO, "playback finished");
        mediaStateChanged(MediaState::STOPPED);
    });
    return true;
}

bool DefaultAudioOutput::stop() {
    log(logger::LoggerHandler::Level::INFO, "stop");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_playing = false;
    m_paused = false;
    m_cv.notify_one();
    return true;
}

bool DefaultAudioOutput::pause() {
    log(logger::LoggerHandler::Level::INFO, "pause");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_paused = true;
    mediaStateChanged(MediaState::STOPPED);
    m_cv.notify_one();
    return true;
}

bool DefaultAudioOutput::resume() {
    log(logger::LoggerHandler::Level::INFO, "resume");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_paused = false;
    mediaStateChanged(MediaState::PLAYING);
    m_cv.notify_one();
    return true;
}

int64_t DefaultAudioOutput::getPosition() {
    log(logger::LoggerHandler::Level::INFO, "getPosition");
    return m_position;
}

bool DefaultAudioOutput::setPosition(int64_t position) {
    std::stringstream ss;
    ss << "setPosition: " << position;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    m_position = position;
    return true;
}

int64_t DefaultAudioOutput::getDuration() {
    log(logger::LoggerHandler::Level::INFO, "getDuration");
    return TIME_UNKNOWN;
}

bool DefaultAudioOutput::volumeChanged(float volume) {
    std::stringstream ss;
    ss << "volumeChanged: " << volume;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    return true;
}

bool DefaultAudioOutput::mutedStateChanged(MutedState state) {
    std::stringstream ss;
    ss << "mutedStateChanged: " << state;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    return true;
}

// private

void DefaultAudioOutput::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "DefaultAudioOutput<" + m_name + ">", message);
}

void DefaultAudioOutput::writeStreamToFile(std::shared_ptr<aace::audio::AudioStream> stream) {
    auto applicationContext = m_applicationContext.lock();
    if (!applicationContext) {
        return;
    }

    auto path = applicationContext->makeTempPath(m_name, "mp3");

    std::stringstream ss;
    ss << "writeStreamToFile: " << path;
    log(logger::LoggerHandler::Level::INFO, ss.str());

    if (std::remove(path.c_str()) == 0) {
        log(logger::LoggerHandler::Level::INFO, "File successfully deleted: " + path);
    }

    auto output = std::make_shared<std::ofstream>(path, std::ios::binary | std::ofstream::out | std::ofstream::app);
    if (!output->good()) {
        log(logger::LoggerHandler::Level::INFO, "Could not create cache file: " + path);
        return;
    }
    char buffer[4096];
    ssize_t bytes = 0;
    ssize_t count;
    while (!stream->isClosed()) {
        count = stream->read(buffer, 4096);
        if (count > 0) {
            bytes += count;
            output->write(buffer, count);
        }
    }
    output->close();

    ss.clear();
    ss << "Cache file complete: " << path << " size: " << bytes;
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

}  // namespace audio
}  // namespace sampleApp
