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

#include "SampleApp/Alexa/SpeechRecognizerHandler.h"

// C++ Standard Library
#include <cstring>
#include <regex>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SpeechRecognizerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

SpeechRecognizerHandler::SpeechRecognizerHandler(std::weak_ptr<Activity> activity,
                                                 std::weak_ptr<logger::LoggerHandler> loggerHandler,
                                                 std::shared_ptr<sampleApp::AudioInputManager> platformAudioCapture,
                                                 bool wakewordDetectionEnabled)
    : aace::alexa::SpeechRecognizer{wakewordDetectionEnabled}
    , m_activity{std::move(activity)}
    , m_loggerHandler{std::move(loggerHandler)}
    , m_platformAudioCapture{std::move(platformAudioCapture)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    Expects(m_platformAudioCapture != nullptr);
    setupUI();
}

std::weak_ptr<Activity> SpeechRecognizerHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> SpeechRecognizerHandler::getLoggerHandler() { return m_loggerHandler; }

bool SpeechRecognizerHandler::startStreamingAudioFile(const std::string &audioFilePath) {
    log(logger::LoggerHandler::Level::INFO, "startStreamingAudioFile:audioFilePath=" + audioFilePath);
    if (m_isStreamingAudioFile.load()) {
        return false;
    }
    if (isWakewordDetectionEnabled()) {
        return streamAudioFile(audioFilePath);
    }
    m_audioFilePath = audioFilePath;
    return tapToTalk();
}

bool SpeechRecognizerHandler::stopStreamingAudioFile() {
    log(logger::LoggerHandler::Level::INFO, "stopStreamingAudioFile");
    if (m_isStreamingAudioFile.load()) {
        m_shouldStopStreamingAudioFile = true;
        if (m_streamTask.valid()) {
            m_streamTask.get();
        }
    }
    return true;
}

bool SpeechRecognizerHandler::streamAudioFile(const std::string &audioFilePath) {
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }
    auto executor = activity->getExecutor();
    if (!executor) {
        return false;
    }
    std::shared_ptr<AudioFileReader> stream = std::make_shared<AudioFileReader>(audioFilePath);
    if (!stream->open()) {
        log(logger::LoggerHandler::Level::ERROR, "Error: error opening input stream: " + audioFilePath);
        return false;
    }
    m_isStreamingAudioFile = true;
    m_shouldStopStreamingAudioFile = false;
    m_streamTask = executor->submit([=]() {
        const size_t size = 160;
        int16_t samples[size];
        size_t count = 0;
        while ((count = stream->read(samples, size)) != 0) {
            if (m_shouldStopStreamingAudioFile.load()) {
                break;
            }
            if (write(samples, count) < 0) {
                break;
            }
        }
        stream->close();
        std::memset(samples, 0, size);
        while (!m_shouldStopStreamingAudioFile.load()) {
            if (write(samples, size) < 0) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        m_isStreamingAudioFile = false;
        m_shouldStopStreamingAudioFile = false;
    });
    return true;
}

// aace::alexa::SpeechRecognizer interface

bool SpeechRecognizerHandler::wakewordDetected(const std::string &wakeword) {
    log(logger::LoggerHandler::Level::INFO, "wakewordDetected:wakeword=" + wakeword);
    auto activity = m_activity.lock();
    if (!activity) {
        return false;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("Wakeword detected:", wakeword);
        }
    });
    return true;
}

void SpeechRecognizerHandler::endOfSpeechDetected() {
    log(logger::LoggerHandler::Level::INFO, "endOfSpeechDetected");
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("End of speech detected");
        }
    });
    if (m_isStreamingAudioFile.load()) {
        stopStreamingAudioFile();
    }
}

bool SpeechRecognizerHandler::startAudioInput() {
    log(logger::LoggerHandler::Level::INFO, "startAudioInput");
    if (!m_audioFilePath.empty()) {
        // Special case to support audio file input
        auto audioFilePath = m_audioFilePath;
        m_audioFilePath.clear();
        return streamAudioFile(audioFilePath);
    }
    return m_platformAudioCapture->startAudioInput("SpeechRecognizer", [this](const int16_t *data, const size_t size) {
        if (m_isStreamingAudioFile.load()) {
            // Special case to support audio file input
            return (ssize_t)size;
        }
        return this->write(data, size);
    });
}

bool SpeechRecognizerHandler::stopAudioInput() {
    log(logger::LoggerHandler::Level::INFO, "stopAudioInput");
    return m_platformAudioCapture->stopAudioInput("SpeechRecognizer");
}

// private

void SpeechRecognizerHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "SpeechRecognizerHandler", message);
}

void SpeechRecognizerHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // holdToTalk
    activity->registerObserver(Event::onSpeechRecognizerHoldToTalk, [=](const std::string &) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerHoldToTalk");
        return holdToTalk();
    });

    // tapToTalk
    activity->registerObserver(Event::onSpeechRecognizerTapToTalk, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerTapToTalk:" + value);
        return tapToTalk();
    });

    // startCapture
    activity->registerObserver(Event::onSpeechRecognizerStartCapture, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerStartCapture:" + value);
        // clang-format off
        static const std::map<std::string, Initiator> InitiatorEnumerator{
            {"HOLD_TO_TALK", Initiator::HOLD_TO_TALK},
            {"TAP_TO_TALK", Initiator::TAP_TO_TALK},
            // {"WAKEWORD", Initiator::WAKEWORD}
        };
        // clang-format on
        if (InitiatorEnumerator.count(value) == 0) {
            return false;
        }
        return startCapture(InitiatorEnumerator.at(value));
    });

    // stopCapture
    activity->registerObserver(Event::onSpeechRecognizerStopCapture, [=](const std::string &) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerStopCapture");
        return stopCapture();
    });

    // enableWakewordDetection
    activity->registerObserver(Event::onSpeechRecognizerEnableWakewordDetection, [=](const std::string &) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerEnableWakewordDetection");
        return enableWakewordDetection();
    });

    // disableWakewordDetection
    activity->registerObserver(Event::onSpeechRecognizerDisableWakewordDetection, [=](const std::string &) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerDisableWakewordDetection");
        return disableWakewordDetection();
    });

    // startStreamingAudioFile
    activity->registerObserver(Event::onSpeechRecognizerStartStreamingAudioFile, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerStartStreamingAudioFile:" + value);
        return startStreamingAudioFile(value);
    });

    // stopStreamingAudioFile
    activity->registerObserver(Event::onSpeechRecognizerStopStreamingAudioFile, [=](const std::string &) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerStopStreamingAudioFile");
        return stopStreamingAudioFile();
    });
}

} // namespace alexa
} // namespace sampleApp
