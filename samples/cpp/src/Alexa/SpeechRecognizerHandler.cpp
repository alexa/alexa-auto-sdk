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

#include "SampleApp/Alexa/SpeechRecognizerHandler.h"

#include <AASB/Message/Alexa/SpeechRecognizer/StopCaptureMessage.h>
#include <AASB/Message/Alexa/SpeechRecognizer/EndOfSpeechDetectedMessage.h>
#include <AASB/Message/Alexa/SpeechRecognizer/WakewordDetectedMessage.h>

// C++ Standard Library
#include <cstring>
#include <regex>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::speechRecognizer;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SpeechRecognizerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// aliases
using MessageBroker = aace::core::MessageBroker;

SpeechRecognizerHandler::SpeechRecognizerHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::weak_ptr<propertyManager::PropertyManagerHandler> propertyManagerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_propertyManagerHandler{std::move(propertyManagerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr) &&
    //        (m_propertyManagerHandler != nullptr) && (m_messageBroker != nullptr));
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> SpeechRecognizerHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> SpeechRecognizerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void SpeechRecognizerHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to EndOfSpeechDetected Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleEndOfSpeechDetectedMessage(message); },
        EndOfSpeechDetectedMessage::topic(),
        EndOfSpeechDetectedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to WakewordDetected Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleWakewordDetectedMessage(message); },
        WakewordDetectedMessage::topic(),
        WakewordDetectedMessage::action());
}

void SpeechRecognizerHandler::handleEndOfSpeechDetectedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received EndOfSpeechDetectedMessage");
    endOfSpeechDetected();
}

void SpeechRecognizerHandler::handleWakewordDetectedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received WakewordDetectedMessage");
    WakewordDetectedMessage msg = json::parse(message);
    wakewordDetected(msg.payload.wakeword);
}

void SpeechRecognizerHandler::wakewordDetected(const std::string& wakeword) {
    log(logger::LoggerHandler::Level::INFO, "wakewordDetected:wakeword=" + wakeword);
    auto activity = m_activity.lock();
    if (activity) {
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printLine("Wakeword detected:", wakeword);
            }
        });
    }
}

void SpeechRecognizerHandler::endOfSpeechDetected() {
    log(logger::LoggerHandler::Level::INFO, "endOfSpeechDetected");
    auto activity = m_activity.lock();
    if (activity) {
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printLine("End of speech detected");
            }
        });
    }
}

void SpeechRecognizerHandler::startCapture(Initiator initiator) {
    // Publish the "StartCapture" message
    StartCaptureMessage msg;
    msg.payload.initiator = initiator;

    // the actual implementation must populate these attributes if the initiator type is WAKEWORD
    // specifying default values for TAP_TO_TALK and HOLD_TO_TALK initiator types.
    msg.payload.keywordBegin = -1;
    msg.payload.keywordEnd = -1;
    msg.payload.keyword = "";

    m_messageBroker->publish(msg.toString());
}

void SpeechRecognizerHandler::stopCapture() {
    // Publish the "StopCapture" message
    StopCaptureMessage msg;
    m_messageBroker->publish(msg.toString());
}

void SpeechRecognizerHandler::holdToTalk() {
    startCapture(Initiator::HOLD_TO_TALK);
}

void SpeechRecognizerHandler::tapToTalk() {
    startCapture(Initiator::TAP_TO_TALK);
}

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
        holdToTalk();
        return true;
    });

    // tapToTalk
    activity->registerObserver(Event::onSpeechRecognizerTapToTalk, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerTapToTalk:" + value);
        tapToTalk();
        return true;
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
        startCapture(InitiatorEnumerator.at(value));
        return true;
    });

    // stopCapture
    activity->registerObserver(Event::onSpeechRecognizerStopCapture, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onSpeechRecognizerStopCapture");
        stopCapture();
        return true;
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
