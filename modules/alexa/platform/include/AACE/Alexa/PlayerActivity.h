/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_PLAYER_ACTIVITY_H
#define AACE_ALEXA_PLAYER_ACTIVITY_H

#include <iostream>

/** @file */

/**
 * The @c aace namespace.
 */
namespace aace {

/**
 * The @c alexa namespace.
 */
namespace alexa {

/**
 * An enum class used to specify player activity for AudioPlayer and audio player state for TemplateRuntime.
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

inline std::ostream& operator<<(std::ostream& stream, const PlayerActivity& activity) {
    switch (activity) {
        case PlayerActivity::IDLE:
            stream << "IDLE";
            break;
        case PlayerActivity::PLAYING:
            stream << "PLAYING";
            break;
        case PlayerActivity::STOPPED:
            stream << "STOPPED";
            break;
        case PlayerActivity::PAUSED:
            stream << "PAUSED";
            break;
        case PlayerActivity::BUFFER_UNDERRUN:
            stream << "BUFFER_UNDERRUN";
            break;
        case PlayerActivity::FINISHED:
            stream << "FINISHED";
            break;
    }
    return stream;
}

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_PLAYER_ACTIVITY_H
