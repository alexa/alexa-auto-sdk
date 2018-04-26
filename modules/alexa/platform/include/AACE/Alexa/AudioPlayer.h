/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 * The @c AudioPlayer class should be extended by the platform implementation to handle audio playback from AVS.
 */
class AudioPlayer : public AudioChannel {
protected:
    AudioPlayer( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker );

public:
    virtual ~AudioPlayer() = default;

    /// Identifies the player state.
    enum class PlayerActivity {
        /// Initial state, prior to acting on the first @c Play directive.
        IDLE,
        /// Indicates that audio is currently playing.
        PLAYING,
        /// Indicates that audio playback was stopped due to an error or a directive which stops or replaces the current stream.
        STOPPED,
        /// Indicates that the audio stream has been paused.
        PAUSED,
        /// Indicates that a buffer underrun has occurred and the stream is buffering.
        BUFFER_UNDERRUN,
        /// Indicates that playback has finished.
        FINISHED
    };

    /**
     * Called when the platform implementation should handle AVS player activity state changes.
     *
     * @param [in] state The new player activity state.
     * @sa PlayerActivity
     */
    virtual void playerActivityChanged( PlayerActivity state ) {}
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_AUDIO_PLAYER_H
