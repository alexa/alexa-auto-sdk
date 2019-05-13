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
 */

#include "SampleApp/Communication/CommunicationHandler.h"

// C++ Standard Library
#include <regex>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace communication {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CommunicationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

CommunicationHandler::CommunicationHandler(std::weak_ptr<Activity> activity,
                                           std::weak_ptr<logger::LoggerHandler> loggerHandler,
                                           std::shared_ptr<sampleApp::AudioInputManager> audioInputChannel,
                                           std::shared_ptr<aace::alexa::MediaPlayer> ringtoneMediaPlayer,
                                           std::shared_ptr<aace::alexa::Speaker> ringtoneSpeaker,
                                           std::shared_ptr<aace::alexa::MediaPlayer> callAudioMediaPlayer,
                                           std::shared_ptr<aace::alexa::Speaker> callAudioSpeaker)
    : aace::communication::AlexaComms{ringtoneMediaPlayer, ringtoneSpeaker, callAudioMediaPlayer, callAudioSpeaker}
    , m_activity{std::move(activity)}
    , m_loggerHandler{std::move(loggerHandler)}
    , m_callState{CallState::NONE}
    , m_audioInputChannel{std::move(audioInputChannel)} {
    setupUI();
}

std::weak_ptr<Activity> CommunicationHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> CommunicationHandler::getLoggerHandler() { return m_loggerHandler; }

// aace::communication::AlexaComms interface
void CommunicationHandler::callStateChanged(CallState state) {
    log(logger::LoggerHandler::Level::VERBOSE, "callStateChanged:" + callStateToString(state));
    // Update call state
    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine("Communication call state changed from " + callStateToString(m_callState) + " to " + callStateToString(state));
        console->printRuler();
    }
    m_callState = state;

    // Hook in the microphone
    if (state == CallState::CALL_CONNECTED) {
        startAudioInput();
    } else {
        stopAudioInput();
    }
}

void CommunicationHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // onCommunicationAcceptCall
    activity->registerObserver(Event::onCommunicationAcceptCall, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onCommunicationAcceptCall");

        if (m_callState == CallState::INBOUND_RINGING) {
            acceptCall();
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Communication call accepted");
                console->printRuler();
            }
        } else if (m_callState == CallState::CALL_CONNECTED) {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Communication call is already connected");
                console->printRuler();
            }
        } else {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("There is no current call to accept");
                console->printRuler();
            }
        }

        return true;
    });

    // onCommunicationStopCall
    activity->registerObserver(Event::onCommunicationStopCall, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onCommunicationStopCall");

        if (m_callState == CallState::CALL_CONNECTED || m_callState == CallState::INBOUND_RINGING) {
            stopCall();
            stopAudioInput();
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Communication call stopped");
                console->printRuler();
            }
        } else {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("There is no current call to stop");
                console->printRuler();
            }
        }

        return true;
    });

    // onCommunicationShowState
    activity->registerObserver(Event::onCommunicationShowState, [=](const std::string &value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onCommunicationShowState");
        showState();
        return true;
    });
}

void CommunicationHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "CommunicationHandler", message);
}

void CommunicationHandler::showState() {
    log(logger::LoggerHandler::Level::VERBOSE, "Showing communications state");
    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine("Communication call state: " + callStateToString(m_callState));
        console->printRuler();
    }
}

bool CommunicationHandler::startAudioInput() {
    log(logger::LoggerHandler::Level::INFO, "startAudioInput");
    return m_audioInputChannel->startAudioInput("Comms", [this](const int16_t *data, const size_t size) {
        int16_t *nc_data = const_cast<int16_t *>(data);
        return writeMicrophoneAudioData(nc_data, size);
    });
}

bool CommunicationHandler::stopAudioInput() {
    log(logger::LoggerHandler::Level::INFO, "stopAudioInput");
    return m_audioInputChannel->stopAudioInput("Comms");
}

std::string CommunicationHandler::callStateToString(CallState state) {
    static const std::map<CallState, std::string> callStateMap{{CallState::CONNECTING, "CONNECTING"},
                                                               {CallState::INBOUND_RINGING, "INBOUND_RINGING"},
                                                               {CallState::CALL_CONNECTED, "CALL_CONNECTED"},
                                                               {CallState::CALL_DISCONNECTED, "CALL_DISCONNECTED"},
                                                               {CallState::NONE, "NONE"}};

    return callStateMap.at(state);
}

} // namespace communication
} // namespace sampleApp
