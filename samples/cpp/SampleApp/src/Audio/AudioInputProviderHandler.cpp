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

#include "SampleApp/Audio/AudioInputProviderHandler.h"

// C++ Standard Library
#include <sstream>
#include <array>
#include <cstring>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace audio {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioInputProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AudioInputProviderHandler::AudioInputProviderHandler(
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

std::weak_ptr<Activity> AudioInputProviderHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AudioInputProviderHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::audio::AudioInputProvider interface

std::shared_ptr<aace::audio::AudioInput> AudioInputProviderHandler::openChannel(
    const std::string& name,
    AudioInputType type) {
    if (type == AudioInputType::VOICE || type == AudioInputType::COMMUNICATION) {
        if (m_sharedAudioInput == nullptr) {
            m_sharedAudioInput = DefaultAudioInput::create();
        }

        return m_sharedAudioInput;
    } else {
        return nullptr;
    }
}

// private

void AudioInputProviderHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AudioInputProviderHandler", message);
}

void AudioInputProviderHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
    activity->registerObserver(Event::onSpeechRecognizerStartStreamingAudioFile, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerStartStreamingAudioFile:" + value);
        m_sharedAudioInput->setStream(FileAudioStream::create(value));
        return true;
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DefaultAudioInput
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#define NUM_SAMPLES 160

bool DefaultAudioInput::setStream(std::shared_ptr<aace::audio::AudioStream> stream) {
    if (m_stream != nullptr) {
        return false;
    }
    m_stream = stream;
    return true;
}

bool DefaultAudioInput::startAudioInput() {
    int16_t buffer[NUM_SAMPLES] = {0};
    size_t bsize = NUM_SAMPLES * 2;

    m_running = true;

    m_executer.submit([=]() {
        while (m_running) {
            if (m_stream != nullptr) {
                if (m_stream->isClosed()) {
                    m_stream.reset();
                    std::memset((char*)buffer, 0, bsize);
                } else {
                    ssize_t count = m_stream->read((char*)buffer, bsize);
                    if (count < bsize) {
                        std::memset(((char*)buffer) + count, 0, bsize - count);
                    }
                }
            }

            write(buffer, NUM_SAMPLES);

            // sleep
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    return true;
}

bool DefaultAudioInput::stopAudioInput() {
    m_running = false;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FileAudioStream
//
////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<FileAudioStream> FileAudioStream::create(const std::string& path) {
    auto stream = std::shared_ptr<FileAudioStream>(new FileAudioStream());

    stream->open(path);

    return stream;
}

bool FileAudioStream::open(const std::string& path) {
    m_stream = std::ifstream(path, std::ios::binary);

    return m_stream.is_open();
}

ssize_t FileAudioStream::read(char* data, const size_t size) {
    if (m_stream.eof()) {
        m_closed = true;
        return 0;
    }

    // read the data from the stream
    m_stream.read(data, size);
    if (m_stream.bad()) {
        m_closed = true;
        return 0;
    }

    // get the number of bytes read
    ssize_t count = m_stream.gcount();

    m_stream.tellg();  // Don't remove otherwise the ReseourceStream used for Alerts/Timers won't work as expected.

    return count;
}

bool FileAudioStream::isClosed() {
    return m_closed;
}

}  // namespace audio
}  // namespace sampleApp
