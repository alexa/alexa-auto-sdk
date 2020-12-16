/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/TextToSpeech/TextToSpeechHandler.h"
// C++ Standard Library
#include <sstream>
#include <unistd.h>
#include <utility>
#include <fstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace textToSpeech {

static constexpr size_t READ_BUFFER_SIZE = 4096;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TextToSpeechHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const std::string ID = "TEXT_TO_SPEECH";
static const std::string TEXT_TO_SPEECH_PROVIDER = "text-to-speech-provider";
using json = nlohmann::json;
TextToSpeechHandler::TextToSpeechHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::weak_ptr<audio::AudioOutputProviderHandler> audioOutputProviderHandler) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_audioOutputHandler{std::move(audioOutputProviderHandler)},
        m_idCounter(0) {
    setupUI();
}

std::weak_ptr<logger::LoggerHandler> TextToSpeechHandler::getLoggerHandler() {
    return m_loggerHandler;
}

std::weak_ptr<Activity> TextToSpeechHandler::getActivity() {
    return m_activity;
}

// aace::textToSpeech:TextToSpeech interface
void TextToSpeechHandler::prepareSpeechCompleted(
    const std::string& speechId,
    std::shared_ptr<aace::audio::AudioStream> preparedAudio,
    const std::string& metadata) {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Prepare speech Completed");
            console->printLine("Speech ID      : ", speechId);
            console->printLine("Metadata       : ", metadata);
            console->printRuler();
        }
    });
    // For QA testing only. The OEM application should follow the UX guidelines in
    // order to play the audio
    try {
        auto path = "" + speechId + ".mp3";
        log(logger::LoggerHandler::Level::VERBOSE, "path" + path);

        // create the output file
        auto output = std::make_shared<std::ofstream>(path, std::ios::binary);
        // copy the stream to the file
        char buffer[READ_BUFFER_SIZE];
        ssize_t size = 0;

        while (!preparedAudio->isClosed()) {
            ssize_t bytesRead = preparedAudio->read(buffer, READ_BUFFER_SIZE);

            // throw an error if the read failed
            if (bytesRead < 0) {
                log(logger::LoggerHandler::Level::VERBOSE, "bytes read is zero");
                return;
            }

            // write the data to the output file
            output->write(buffer, bytesRead);

            size += bytesRead;
        }

        output->close();
        log(logger::LoggerHandler::Level::VERBOSE, "complete size" + size);
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, ex.what());
    }
}

void TextToSpeechHandler::prepareSpeechFailed(const std::string& speechId, const std::string& reason) {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Prepare speech Failed");
            console->printLine("Speech ID      : ", speechId);
            console->printLine("Reason       : ", reason);
            console->printRuler();
        }
    });
}

void TextToSpeechHandler::capabilitiesReceived(const std::string& requestId, const std::string& capabilities) {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Capabilities Received");
            console->printLine("Capabilities      : ", capabilities);
            console->printLine("Request ID      : ", requestId);
            console->printRuler();
        }
    });
}

// private

void TextToSpeechHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "TextToSpeechHandler", message);
}

void TextToSpeechHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }

    m_console = activity->findViewById("id:console");

    // prepareSpeech
    activity->registerObserver(Event::onPrepareSpeech, [=](const std::string& value) {
        auto speechId = generateId();
        log(logger::LoggerHandler::Level::VERBOSE, "onPrepareSpeech text: " + value + " speech ID: " + speechId);
        return prepareSpeech(speechId, value, TEXT_TO_SPEECH_PROVIDER, "");
    });

    // getCapabilities
    activity->registerObserver(Event::onGetCapabilities, [=](const std::string& value) {
        auto requestId = generateId();
        log(logger::LoggerHandler::Level::VERBOSE, "onGetCapabilities request ID: " + requestId);
        return getCapabilities(requestId, TEXT_TO_SPEECH_PROVIDER);
    });
}

std::string TextToSpeechHandler::generateId() {
    return ID + "-" + std::to_string(++m_idCounter);
}

}  // namespace textToSpeech
}  // namespace sampleApp
