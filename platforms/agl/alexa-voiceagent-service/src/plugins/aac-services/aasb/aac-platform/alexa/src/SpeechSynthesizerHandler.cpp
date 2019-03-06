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
#include "SpeechSynthesizerHandler.h"

#include <rapidjson/document.h>

#include <aasb/Consts.h>
#include "MediaPlayerHandler.h"
#include "PlatformSpecificLoggingMacros.h"

namespace aasb {
namespace alexa {

using namespace aasb::bridge;

const std::string TAG = "aasb::alexa::SpeechSynthesizerHandler";

std::shared_ptr<SpeechSynthesizerHandler> SpeechSynthesizerHandler::create(
    bool aacePlatformMediaPlayer,
    std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer,
    std::shared_ptr<aace::alexa::Speaker> speaker) {
    auto speechSynthesizerHandler = std::shared_ptr<SpeechSynthesizerHandler>(
        new SpeechSynthesizerHandler(aacePlatformMediaPlayer, mediaPlayer, speaker));

    return speechSynthesizerHandler;
}

SpeechSynthesizerHandler::SpeechSynthesizerHandler(
    bool aacePlatformMediaPlayer,
    std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer,
    std::shared_ptr<aace::alexa::Speaker> speaker) :
        aace::alexa::SpeechSynthesizer(mediaPlayer, speaker),
        m_aacePlatformMediaPlayer(aacePlatformMediaPlayer),
        m_mediaPlayer(mediaPlayer),
        m_speaker(speaker) {
}

void SpeechSynthesizerHandler::onReceivedEvent(const std::string& action, const std::string& payload) {
    if (m_aacePlatformMediaPlayer) {
        AASB_ERROR("%s: action %s cant be executed on AACE Platform media player.", TAG.c_str(), action.c_str());
        return;
    }

    auto mediaPlayer = std::static_pointer_cast<MediaPlayerHandler>(m_mediaPlayer);

    if (action == ACTION_MEDIA_STATE_CHANGED) {
        mediaPlayer->onMediaStateChangedEvent(payload);
        return;
    }

    if (action == ACTION_MEDIA_ERROR) {
        rapidjson::Document document;
        document.Parse(payload.c_str());
        auto root = document.GetObject();

        std::string mediaError = "";
        if (root.HasMember("mediaError") && root["mediaError"].IsString()) mediaError = root["mediaError"].GetString();

        std::string description = "";
        if (root.HasMember("description") && root["description"].IsString())
            description = root["description"].GetString();

        if (!mediaError.empty()) mediaPlayer->onMediaErrorEvent(mediaError, description);
        return;
    }

    if (action == ACTION_MEDIA_PLAYER_POSITION) {
        mediaPlayer->onMediaPlayerPositionReceived(std::stoi(payload));
        return;
    }

    AASB_ERROR("%s: action %s is unknown.", TAG.c_str(), action.c_str());
}
}  // namespace alexa
}  // namespace aasb