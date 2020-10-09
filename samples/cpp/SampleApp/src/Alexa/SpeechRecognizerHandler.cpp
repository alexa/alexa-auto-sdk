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

SpeechRecognizerHandler::SpeechRecognizerHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::weak_ptr<propertyManager::PropertyManagerHandler> propertyManagerHandler) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_propertyManagerHandler{std::move(propertyManagerHandler)} {
    setupUI();
}

std::weak_ptr<Activity> SpeechRecognizerHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> SpeechRecognizerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::SpeechRecognizer interface

bool SpeechRecognizerHandler::wakewordDetected(const std::string& wakeword) {
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
}

// private

void SpeechRecognizerHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
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
    activity->registerObserver(Event::onSpeechRecognizerHoldToTalk, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerHoldToTalk");
        return holdToTalk();
    });

    // tapToTalk
    activity->registerObserver(Event::onSpeechRecognizerTapToTalk, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerTapToTalk:" + value);
        return tapToTalk();
    });

    // startCapture
    activity->registerObserver(Event::onSpeechRecognizerStartCapture, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerStartCapture:" + value);
        // clang-format off
        static const std::map<std::string, Initiator> InitiatorEnumerator{
            {"HOLD_TO_TALK", Initiator::HOLD_TO_TALK},
            {"TAP_TO_TALK", Initiator::TAP_TO_TALK}
        };
        // clang-format on
        if (InitiatorEnumerator.count(value) == 0) {
            return false;
        }
        return startCapture(InitiatorEnumerator.at(value));
    });

    // stopCapture
    activity->registerObserver(Event::onSpeechRecognizerStopCapture, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerStopCapture");
        return stopCapture();
    });

    // enableWakewordDetection
    activity->registerObserver(Event::onSpeechRecognizerEnableWakewordDetection, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerEnableWakewordDetection");
        auto propertyManagerHandler = m_propertyManagerHandler.lock();
        if (propertyManagerHandler == nullptr) {
            log(logger::LoggerHandler::Level::ERROR, "nullPropertyManagerHandler");
            return false;
        }
        if (propertyManagerHandler->setProperty(aace::alexa::property::WAKEWORD_ENABLED, "true")) {
            return true;
        }
        log(logger::LoggerHandler::Level::ERROR, "Enable Wakeword Detection failed");
        return false;
    });

    // disableWakewordDetection
    activity->registerObserver(Event::onSpeechRecognizerDisableWakewordDetection, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerDisableWakewordDetection");
        auto propertyManagerHandler = m_propertyManagerHandler.lock();
        if (propertyManagerHandler == nullptr) {
            log(logger::LoggerHandler::Level::ERROR, "nullPropertyManagerHandler");
            return false;
        }
        if (propertyManagerHandler->setProperty(aace::alexa::property::WAKEWORD_ENABLED, "false")) {
            return true;
        }
        log(logger::LoggerHandler::Level::ERROR, "Disable Wakeword Detection failed");
        return false;
    });
}

}  // namespace alexa
}  // namespace sampleApp
