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
#include "AudioPlayerHandler.h"

#include <rapidjson/document.h>

#include <aasb/Consts.h>
#include "PlatformSpecificLoggingMacros.h"
using Level = aace::logger::LoggerEngineInterface::Level;
namespace aasb {
namespace alexa {

using namespace aasb::bridge;

const std::string TAG = "aasb::alexa::AudioPlayerHandler";

std::shared_ptr<aasb::alexa::AudioPlayerHandler> AudioPlayerHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger) {
    auto audioPlayerHandler = std::shared_ptr<aasb::alexa::AudioPlayerHandler>(
        new AudioPlayerHandler(logger));

    return audioPlayerHandler;
}

AudioPlayerHandler::AudioPlayerHandler(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger) :
        aace::alexa::AudioPlayer(),
        m_logger(logger) {
}

void AudioPlayerHandler::onReceivedEvent(const std::string& action, const std::string& payload) {
    if (action == ACTION_PLAYER_ACTIVITY) {
        m_logger->log(Level::INFO, TAG, "Received event: Player Activity Changed ");
        rapidjson::Document document;
        document.Parse(payload.c_str());
        auto root = document.GetObject();

        std::string playerActivityStr = "";
        if (root.HasMember("playerActivity") && root["playerActivity"].IsString()) {
            std::string playerActivityStr = root["playerActivity"].GetString();
            if (playerActivityStr == VALUE_PLAYERACTIVITY_IDLE) {
                playerActivityChanged(PlayerActivity::IDLE);
            } else if (playerActivityStr == VALUE_PLAYERACTIVITY_PLAYING) {
                playerActivityChanged(PlayerActivity::PLAYING);
            } else if (playerActivityStr == VALUE_PLAYERACTIVITY_STOPPED) {
                playerActivityChanged(PlayerActivity::STOPPED);
            } else if (playerActivityStr == VALUE_PLAYERACTIVITY_PAUSED) {
                playerActivityChanged(PlayerActivity::PAUSED);
            } else if (playerActivityStr == VALUE_PLAYERACTIVITY_BUFFER_UNDERRUN) {
                playerActivityChanged(PlayerActivity::BUFFER_UNDERRUN);
            } else if (playerActivityStr == VALUE_PLAYERACTIVITY_FINISHED) {
                playerActivityChanged(PlayerActivity::FINISHED);
            } else {
                m_logger->log(Level::WARN, TAG, "playerActivity: Invalid args " + payload);
            }
        }

    }

   m_logger->log(Level::WARN, TAG, "onReceivedEvent: Unknown action " + action);
}

 
}  // namespace alexa
}  // namespace aasb
