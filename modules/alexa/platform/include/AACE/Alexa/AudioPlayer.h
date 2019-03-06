/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_AUDIO_PLAYER_H
#define AACE_ALEXA_AUDIO_PLAYER_H

#include "AudioChannel.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * AudioPlayer should be extended to handle audio output from the Engine.
 *
 * The AudioPlayer @c MediaPlayer and @c Speaker will receive directives from the Engine to handle audio playback.
 *
 * @note Audio playback control operations such as on-platform button presses must be routed through
 * the @c PlaybackController.
 *
 * @sa AudioChannel
 * @sa PlaybackController
 * @sa TemplateRuntime::renderPlayerInfo()
 */
class AudioPlayer : public AudioChannel {
protected:
    AudioPlayer( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker );

public:
    virtual ~AudioPlayer();

    /**
     * Specifies the state of audio playback activity
     */
    enum class PlayerActivity {

        /**
         * Audio playback has not yet begun.
         */
        IDLE,

        /**
         * Audio is currently playing.
         */
        PLAYING,

        /**
         * Audio playback is stopped, either from a stop directive or playback error.
         */
        STOPPED,

        /**
         * Audio playback is paused.
         */
        PAUSED,

        /**
         * Audio playback is stalled because a buffer underrun has occurred.
         */
        BUFFER_UNDERRUN,

        /**
         * Audio playback is finished.
         */
        FINISHED
    };

    /**
     * Notifies the platform implementation of a change in audio playback state
     *
     * @param [in] state The new playback state
     */
    virtual void playerActivityChanged( PlayerActivity state ) {}
};

inline std::ostream& operator<<(std::ostream& stream, const AudioPlayer::PlayerActivity& state) {
    switch (state) {
        case AudioPlayer::PlayerActivity::IDLE:
            stream << "IDLE";
            break;
        case AudioPlayer::PlayerActivity::PLAYING:
            stream << "PLAYING";
            break;
        case AudioPlayer::PlayerActivity::STOPPED:
            stream << "STOPPED";
            break;
        case AudioPlayer::PlayerActivity::PAUSED:
            stream << "PAUSED";
            break;
        case AudioPlayer::PlayerActivity::BUFFER_UNDERRUN:
            stream << "BUFFER_UNDERRUN";
            break;
        case AudioPlayer::PlayerActivity::FINISHED:
            stream << "FINISHED";
            break;
    }
    return stream;
}

} // aace::alexa
} // aace

#endif // AACE_ALEXA_AUDIO_PLAYER_H
