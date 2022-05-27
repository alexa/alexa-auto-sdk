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

#include "SampleApp/Audio/AudioInputProviderHandler.h"

#include <AASB/Message/Audio/AudioInput/StopAudioInputMessage.h>
#include <AASB/Message/Audio/AudioInput/StartAudioInputMessage.h>

// C++ Standard Library
#include <sstream>
#include <array>
#include <cstring>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace audio {

using namespace aasb::message::audio::audioInput;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioInputProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using aace::core::MessageBroker;
using aace::core::MessageStream;

AudioInputProviderHandler::AudioInputProviderHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker,
    bool setup) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    if (setup) {
        setupUI();
    }
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> AudioInputProviderHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AudioInputProviderHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void AudioInputProviderHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to StartAudioInput Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleStartAudioInputMessage(message); },
        StartAudioInputMessage::topic(),
        StartAudioInputMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to StopAudioInput Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleStopAudioInputMessage(message); },
        StopAudioInputMessage::topic(),
        StopAudioInputMessage::action());
}

void AudioInputProviderHandler::handleStartAudioInputMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received StartAudioInputMessage");

    StartAudioInputMessage msg = json::parse(message);
    auto stream = m_messageBroker->openStream(msg.payload.streamId, MessageStream::Mode::WRITE);
    startAudioInput(stream);
}

void AudioInputProviderHandler::handleStopAudioInputMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received StopAudioInputMessage");
    stopAudioInput();
}

#define NUM_SAMPLES 160

void AudioInputProviderHandler::startAudioInput(std::shared_ptr<MessageStream> stream) {
    int16_t buffer[NUM_SAMPLES] = {0};
    size_t bsize = NUM_SAMPLES * 2;

    m_running = true;

    if (m_stream.is_open()) {
        m_stream.close();
    }
    m_stream.open(m_path, std::ios::binary);

    m_executer.submit([=]() {
        while (m_running) {
            if (stream != nullptr) {
                if (stream->isClosed()) {
                    std::memset((char*)buffer, 0, bsize);
                } else {
                    ssize_t count = read((char*)buffer, bsize);
                    if (count < bsize) {
                        std::memset(((char*)buffer) + count, 0, bsize - count);
                    }
                }
            }
            if (!stream->isClosed()) {
                stream->write((char*)buffer, NUM_SAMPLES);
            }

            // sleep
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
}

void AudioInputProviderHandler::stopAudioInput() {
    m_running = false;
}

void AudioInputProviderHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AudioInputProviderHandler", message);
}

ssize_t AudioInputProviderHandler::read(char* data, const size_t size) {
    if (m_stream.eof()) {
        return 0;
    }

    // read the data from the stream
    m_stream.read(data, size);
    if (m_stream.bad()) {
        return 0;
    }

    // get the number of bytes read
    ssize_t count = m_stream.gcount();

    m_stream.tellg();

    return count;
}

void AudioInputProviderHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
    activity->registerObserver(Event::onSpeechRecognizerStartStreamingAudioFile, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerStartStreamingAudioFile:" + value);
        m_path = value;
        return true;
    });
}

}  // namespace audio
}  // namespace sampleApp
