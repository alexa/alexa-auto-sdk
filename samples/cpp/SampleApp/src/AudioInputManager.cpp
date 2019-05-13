/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 *
 */

#include "SampleApp/AudioInputManager.h"

namespace sampleApp {

AudioInputManager::AudioInputManager(std::weak_ptr<Activity> activity,
                                     std::weak_ptr<logger::LoggerHandler> loggerHandler,
                                     std::shared_ptr<aace::audio::AudioCapture> inputChannel)
    : m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)}, m_inputChannel{std::move(inputChannel)}, m_capturing{false} {}

bool AudioInputManager::startAudioInput(const std::string &name, const MicrophoneDataHandler &listener) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_observers.find(name) == m_observers.end()) {
        log(logger::LoggerHandler::Level::INFO, "Audio input observer added " + name);
        m_observers[name] = listener;
    } else {
        log(logger::LoggerHandler::Level::INFO, "Audio input observer exists " + name);
        return false;
    }

    if (!m_capturing) {
        return startAudioInput();
    } else {
        log(logger::LoggerHandler::Level::INFO, "Audio input is already in progress");
    }

    return true;
}

auto AudioInputManager::stopAudioInput(const std::string &name) -> bool {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_observers.find(name) == m_observers.end()) {
        log(logger::LoggerHandler::Level::ERROR, "Audio input observer not found " + name);
        return false;
    } else {
        log(logger::LoggerHandler::Level::INFO, "Audio input observer removed " + name);
        m_observers.erase(name);
    }

    if (m_observers.size() == 0) {
        log(logger::LoggerHandler::Level::INFO, "Audio input capture stopped " + name);
        m_capturing = false;
        return m_inputChannel->stopAudioInput();
    } else {
        log(logger::LoggerHandler::Level::INFO, "Audio input capture will continue after removing " + name);
    }

    return true;
}

auto AudioInputManager::startAudioInput() -> bool {
    auto activity = m_activity.lock();
    if (!activity) {
        log(logger::LoggerHandler::Level::ERROR, "Failed to lock activity");
        return false;
    }
    auto executor = activity->getExecutor();
    if (!executor) {
        log(logger::LoggerHandler::Level::ERROR, "Failed to get executor service");
        return false;
    }

    executor->submit([this]() {
        m_capturing = true;
        log(logger::LoggerHandler::Level::INFO, "Audio input started");

        m_inputChannel->startAudioInput([this](const int16_t *data, const size_t size) { return notifyDataAvailable(data, size); });
    });

    return true;
}

auto AudioInputManager::notifyDataAvailable(const int16_t *data, const size_t size) -> bool {
    for (auto it = m_observers.begin(); it != m_observers.end(); ++it) {
        it->second(data, size);
    }

    return true;
}

void AudioInputManager::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AudioInputManager", message);
}

} // namespace sampleApp
