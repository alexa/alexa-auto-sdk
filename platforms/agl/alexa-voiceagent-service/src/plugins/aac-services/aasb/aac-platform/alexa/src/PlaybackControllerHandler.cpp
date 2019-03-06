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

#include "PlaybackControllerHandler.h"

#include <rapidjson/document.h>

#include <aasb/Consts.h>

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

// Namespaces.
using namespace aasb::bridge;

// Logging tag for this file.
static const std::string TAG = "aasb::alexa::PlaybackController";

namespace aasb {
namespace alexa {

std::shared_ptr<PlaybackControllerHandler> PlaybackControllerHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger) {
    return std::shared_ptr<PlaybackControllerHandler>(new PlaybackControllerHandler(logger));
}

PlaybackControllerHandler::PlaybackControllerHandler(std::shared_ptr<aasb::core::logger::LoggerHandler> logger) :
        m_logger(logger) {

}

void PlaybackControllerHandler::onReceivedEvent(const std::string& action, const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "Processing event: " + action + " payload: " + payload);

    if (action == ACTION_PLAYBACK_BUTTON_PRESSED) {
        buttonPressed(payload);
    } else if (action == ACTION_PLAYBACK_TOGGLE_PRESSED) {
        togglePressed(payload);
    }
}

void PlaybackControllerHandler::buttonPressed(const std::string& jsonPayload) {
    m_logger->log(Level::VERBOSE, TAG, "buttonPressed payload " + jsonPayload);

    rapidjson::Document document;
    document.Parse(jsonPayload.c_str());
    auto root = document.GetObject();

    if (root.HasMember(JSON_ATTR_PLAYBACK_BUTTON_TYPE.c_str()) &&
         root[JSON_ATTR_PLAYBACK_BUTTON_TYPE.c_str()].IsString()) {
        std::string buttonTypeStr = root[JSON_ATTR_PLAYBACK_BUTTON_TYPE.c_str()].GetString();

        if (buttonTypeStr == VALUE_PLAYBACK_BUTTON_PLAY) {
            PlaybackController::buttonPressed(PlaybackButton::PLAY);
        } else if (buttonTypeStr == VALUE_PLAYBACK_BUTTON_PAUSE) {
            PlaybackController::buttonPressed(PlaybackButton::PAUSE);
        } else if (buttonTypeStr == VALUE_PLAYBACK_BUTTON_NEXT) {
            PlaybackController::buttonPressed(PlaybackButton::NEXT);
        } else if (buttonTypeStr == VALUE_PLAYBACK_BUTTON_PREVIOUS) {
            PlaybackController::buttonPressed(PlaybackButton::PREVIOUS);
        } else if (buttonTypeStr == VALUE_PLAYBACK_BUTTON_SKIP_FORWARD) {
            PlaybackController::buttonPressed(PlaybackButton::SKIP_FORWARD);
        } else if (buttonTypeStr == VALUE_PLAYBACK_BUTTON_SKIP_BACKWARD) {
            PlaybackController::buttonPressed(PlaybackButton::SKIP_BACKWARD);
        } else {
            m_logger->log(Level::WARN, TAG, "buttonPressed: unrecognized button type: " + buttonTypeStr);
        }
    } else {
        m_logger->log(Level::WARN, TAG, "buttonPressed: button type attribute not found");
    }
}

void PlaybackControllerHandler::togglePressed(const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "togglePressed payload " + payload);

    rapidjson::Document document;
    document.Parse(payload.c_str());
    auto root = document.GetObject();

    PlaybackToggle toggle;
    bool selected = false;

    if (root.HasMember(JSON_ATTR_PLAYBACK_TOGGLE_TYPE.c_str()) &&
         root[JSON_ATTR_PLAYBACK_TOGGLE_TYPE.c_str()].IsString()) {
        std::string toggleTypeStr = root[JSON_ATTR_PLAYBACK_TOGGLE_TYPE.c_str()].GetString();

        if (toggleTypeStr == VALUE_PLAYBACK_TOGGLE_SHUFFLE) {
            toggle = PlaybackToggle::SHUFFLE;
        } else if (toggleTypeStr == VALUE_PLAYBACK_TOGGLE_LOOP) {
            toggle = PlaybackToggle::LOOP;
        } else if (toggleTypeStr == VALUE_PLAYBACK_TOGGLE_REPEAT) {
            toggle = PlaybackToggle::REPEAT;
        } else if (toggleTypeStr == VALUE_PLAYBACK_TOGGLE_THUMBS_DOWN) {
            toggle = PlaybackToggle::THUMBS_DOWN;
        } else if (toggleTypeStr == VALUE_PLAYBACK_TOGGLE_THUMBS_UP) {
            toggle = PlaybackToggle::THUMBS_UP;
        } else {
            m_logger->log(Level::WARN, TAG, "togglePressed: unrecognized toggle type: " + toggleTypeStr);
            return;
        }
    } else {
        m_logger->log(Level::WARN, TAG, "togglePressed: toggle type attribute not found");
        return;
    }

    if (root.HasMember(JSON_ATTR_PLAYBACK_TOGGLE_SELECTED.c_str()) &&
         root[JSON_ATTR_PLAYBACK_TOGGLE_TYPE.c_str()].IsBool()) {
        selected = root[JSON_ATTR_PLAYBACK_TOGGLE_TYPE.c_str()].GetBool();
    } else {
        m_logger->log(Level::WARN, TAG, "togglePressed: toggle selected attribute not found");
        return;
    }

    PlaybackController::togglePressed(toggle, selected);
}

}
}