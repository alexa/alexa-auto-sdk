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

#include "SampleApp/Alexa/PlaybackControllerHandler.h"

// C++ Standard Library
#include <regex>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PlaybackControllerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

PlaybackControllerHandler::PlaybackControllerHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> PlaybackControllerHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> PlaybackControllerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// private

void PlaybackControllerHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "PlaybackControllerHandler", message);
}

void PlaybackControllerHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // buttonPressed
    activity->registerObserver(Event::onPlaybackControllerButtonPressed, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPlaybackControllerButtonPressed:" + value);
        // clang-format off
        static const std::map<std::string, PlaybackButton> PlaybackButtonEnumerator{
            {"PLAY", PlaybackButton::PLAY},
            {"PAUSE", PlaybackButton::PAUSE},
            {"NEXT", PlaybackButton::NEXT},
            {"PREVIOUS", PlaybackButton::PREVIOUS},
            {"SKIP_FORWARD", PlaybackButton::SKIP_FORWARD},
            {"SKIP_BACKWARD", PlaybackButton::SKIP_BACKWARD}
        };
        // clang-format on
        if (PlaybackButtonEnumerator.count(value) == 0) {
            return false;
        }
        buttonPressed(PlaybackButtonEnumerator.at(value));
        return true;
    });

    // togglePressed
    activity->registerObserver(Event::onPlaybackControllerTogglePressed, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onPlaybackControllerTogglePressed:" + value);
        static std::regex r("(.+)/(true|false|1|0)", std::regex::optimize);
        std::smatch sm{};
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 2)) {
            return false;
        }
        // clang-format off
        static const std::map<std::string, PlaybackToggle> PlaybackToggleEnumerator{
            {"SHUFFLE", PlaybackToggle::SHUFFLE},
            {"LOOP", PlaybackToggle::LOOP},
            {"REPEAT", PlaybackToggle::REPEAT},
            {"THUMBS_UP", PlaybackToggle::THUMBS_UP},
            {"THUMBS_DOWN", PlaybackToggle::THUMBS_DOWN}
        };
        // clang-format on
        if (PlaybackToggleEnumerator.count(sm[1]) == 0) {
            return false;
        }
        togglePressed(PlaybackToggleEnumerator.at(sm[1]), sm[2] == "true" || sm[2] == "1");
        return true;
    });
}

}  // namespace alexa
}  // namespace sampleApp
