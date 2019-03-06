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
#ifndef AASB_ALEXA_AUDIOPLAYERHANDLER_H
#define AASB_ALEXA_AUDIOPLAYERHANDLER_H

#include <memory>

#include <AACE/Alexa/AudioPlayer.h>
#include <AACE/Alexa/Speaker.h>

namespace aasb {
namespace alexa {

/**
 * AASB Implementation for @c aace::alexa::AudioPlayer
 */
class AudioPlayerHandler : public aace::alexa::AudioPlayer {
public:
    /**
     * Creates a new instance of @c AudioPlayerHandler.
     *
     * @param aacePlatformMediaPlayer is true if media player is provided by AACE Platform
     *      Audio library. False if media player is an instance of @c MediaPlayerHandler.
     * @param mediaPlayer Media player to use for playing back the audio playback content
     *      received from alexa.
     * @param speaker Speaker to control the volume of @c mediaPlayer.
     */
    static std::shared_ptr<AudioPlayerHandler> create(
        bool aacePlatformMediaPlayer,
        std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer,
        std::shared_ptr<aace::alexa::Speaker> speaker);

    void onReceivedEvent(const std::string& action, const std::string& payload);

private:
    AudioPlayerHandler(
        bool aacePlatformMediaPlayer,
        std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer,
        std::shared_ptr<aace::alexa::Speaker> speaker);

    bool m_aacePlatformMediaPlayer;
    std::shared_ptr<aace::alexa::MediaPlayer> m_mediaPlayer;
    std::shared_ptr<aace::alexa::Speaker> m_speaker;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_AUDIOPLAYERHANDLER_H