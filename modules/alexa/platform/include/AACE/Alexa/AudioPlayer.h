/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Core/PlatformInterface.h>
#include "AlexaEngineInterfaces.h"
#include "PlayerActivity.h"

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
 *
 * @deprecated This platform interface is deprecated.
 *             Use the Alexa Auto Services Bridge (AASB) message broker
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 */
class AudioPlayer : public aace::core::PlatformInterface {
protected:
    AudioPlayer() = default;

public:
    /**
     * Alias the common PlayerActivity namespace.
     */
    using PlayerActivity = aace::alexa::PlayerActivity;

    /**
     * Used when audio time is unknown or indeterminate.
     */
    static const int64_t TIME_UNKNOWN = -1;

    virtual ~AudioPlayer();

    /**
     * Notifies the platform implementation of a change in audio playback state
     *
     * @param [in] state The new playback state
     */
    virtual void playerActivityChanged(PlayerActivity state) {
    }

    /**
     * Returns the current playback position of the audio player.
     * If the audio source is not playing, the most recent position played
     * will be returned.
     *
     * @return The audio player's playback position in milliseconds,
     * or @c TIME_UNKNOWN if the current media position is unknown or invalid.
     */
    int64_t getPlayerPosition();

    /**
     * Returns the playback duration of the audio player.
     *
     * @return The audio player's playback duration in milliseconds,
     * or @c TIME_UNKNOWN if the current media duration is unknown or invalid.
     */
    int64_t getPlayerDuration();

    /**
     * Sets the Alexa @c AudioPlayer interface as the foreground player the user sees on screen. This function is useful 
     * for scenarios in which the user played an @c AudioPlayer media source, then played a different Alexa-aware
     * @c ExternalMediaPlayer media source, such as a deep-linked media app or a local media source, and then manually
     * returned visual activity to the Alexa @c AudioPlayer GUI. Calling this function ensures the next VUI command or
     * GUI interaction with the playback control buttons acts on the @c AudioPlayer source rather than the more recently
     * played @c ExternalMediaPlayer source.
     *
     * @note The @c AudioPlayer had to be previously playing at least once during this Engine cycle in order for this 
     * function to make Alexa act on @c AudioPlayer.
     */
    void setAsForegroundActivity();

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<aace::alexa::AudioPlayerEngineInterface> audioPlayerEngineInterface);

private:
    std::weak_ptr<aace::alexa::AudioPlayerEngineInterface> m_audioPlayerEngineInterface;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_AUDIO_PLAYER_H
