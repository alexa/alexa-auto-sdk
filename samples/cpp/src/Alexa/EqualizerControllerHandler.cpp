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

#include "SampleApp/Alexa/EqualizerControllerHandler.h"

#include <AASB/Message/Alexa/EqualizerController/SetBandLevelsMessage.h>
#include <AASB/Message/Alexa/EqualizerController/GetBandLevelsMessage.h>

#include <AASB/Message/Alexa/EqualizerController/LocalAdjustBandLevelsMessage.h>
#include <AASB/Message/Alexa/EqualizerController/LocalResetBandsMessage.h>
#include <AASB/Message/Alexa/EqualizerController/LocalSetBandLevelsMessage.h>

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::equalizerController;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  EqualizerControllerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// aliases
using MessageBroker = aace::core::MessageBroker;

EqualizerControllerHandler::EqualizerControllerHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr) && (m_messageBroker != nullptr));
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> EqualizerControllerHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> EqualizerControllerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void EqualizerControllerHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetBandLevelsMessage Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetBandLevelsMessage(message); },
        GetBandLevelsMessage::topic(),
        GetBandLevelsMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to SetBandLevelsMessage Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSetBandLevelsMessage(message); },
        SetBandLevelsMessage::topic(),
        SetBandLevelsMessage::action());
}

void EqualizerControllerHandler::handleGetBandLevelsMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetBandLevelsMessage");
    GetBandLevelsMessage msg = json::parse(message);

    // Publish the reply message with the band levels.
    GetBandLevelsMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.bandLevels = getBandLevels();
    m_messageBroker->publish(replyMsg.toString());
}

void EqualizerControllerHandler::handleSetBandLevelsMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received SetBandLevelsMessage");
    SetBandLevelsMessage msg = json::parse(message);
    setBandLevels(msg.payload.bandLevels);
}

void EqualizerControllerHandler::localSetBandLevels(
    const std::vector<aasb::message::alexa::equalizerController::EqualizerBandLevel>& bandLevels) {
    // Publish the LocalSetBandLevelsMessage with the band levels.
    LocalSetBandLevelsMessage msg;
    msg.payload.bandLevels = bandLevels;
    m_messageBroker->publish(msg.toString());
}

void EqualizerControllerHandler::localAdjustBandLevels(
    const std::vector<aasb::message::alexa::equalizerController::EqualizerBandLevel>& bandAdjustments) {
    // Publish the LocalAdjustBandLevelsMessage with the band levels.
    LocalAdjustBandLevelsMessage msg;
    msg.payload.bandAdjustments = bandAdjustments;
    m_messageBroker->publish(msg.toString());
}

void EqualizerControllerHandler::localResetBands(
    const std::vector<aasb::message::alexa::equalizerController::EqualizerBand>& bands) {
    // Publish the LocalResetBandsMessage with the band levels.
    LocalResetBandsMessage msg;
    msg.payload.bands = bands;
    m_messageBroker->publish(msg.toString());
}

std::vector<EqualizerBandLevel> EqualizerControllerHandler::getBandLevels() {
    log(logger::LoggerHandler::Level::INFO, "getBandLevels");
    return {};
}

void EqualizerControllerHandler::setBandLevels(const std::vector<EqualizerBandLevel>& bandLevels) {
    std::stringstream ss;
    char character = '{';
    for (auto& bandLevel : bandLevels) {
        ss << character << toString(bandLevel.band) << ':' << bandLevel.level;
        character = ',';
    }
    ss << '}';
    log(logger::LoggerHandler::Level::INFO, "setBandLevels:bandLevels=" + ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printLine("Set EQ band levels:");
            for (auto& bandLevel : bandLevels) {
                console->printLine(toString(bandLevel.band), bandLevel.level);
            }
        }
    });
}

std::string EqualizerControllerHandler::toString(EqualizerBand enumValue) {
    switch (enumValue) {
        case (EqualizerBand::BASS):
            return "BASS";
        case (EqualizerBand::MIDRANGE):
            return "MIDRANGE";
        case (EqualizerBand::TREBLE):
            return "TREBLE";
    }
    throw std::runtime_error("invalidEqualizerBandType");
}

void EqualizerControllerHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "EqualizerControllerHandler", message);
}

void EqualizerControllerHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // localSetBandLevels
    activity->registerObserver(Event::onEqualizerControllerLocalSetBandLevels, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onEqualizerControllerLocalSetBandLevels");
        return false;
    });

    // localAdjustBandLevels
    activity->registerObserver(Event::onEqualizerControllerLocalAdjustBandLevels, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onEqualizerControllerLocalAdjustBandLevels");
        return false;
    });

    // localResetBands
    activity->registerObserver(Event::onEqualizerControllerLocalResetBands, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onEqualizerControllerLocalResetBands");
        return false;
    });
}

}  // namespace alexa
}  // namespace sampleApp
