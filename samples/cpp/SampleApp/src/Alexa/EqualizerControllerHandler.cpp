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

#include "SampleApp/Alexa/EqualizerControllerHandler.h"

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  EqualizerControllerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

EqualizerControllerHandler::EqualizerControllerHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> EqualizerControllerHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> EqualizerControllerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::EqualizerController interface

std::vector<EqualizerControllerHandler::EqualizerBandLevel> EqualizerControllerHandler::getBandLevels() {
    log(logger::LoggerHandler::Level::INFO, "getBandLevels");
    return {};
}

void EqualizerControllerHandler::setBandLevels(
    const std::vector<EqualizerControllerHandler::EqualizerBandLevel>& bandLevels) {
    std::stringstream ss;
    char character = '{';
    for (auto& bandLevel : bandLevels) {
        ss << character << bandLevel.first << ':' << bandLevel.second;
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
                console->printLine(bandLevel.first, bandLevel.second);
            }
        }
    });
}

// private

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
