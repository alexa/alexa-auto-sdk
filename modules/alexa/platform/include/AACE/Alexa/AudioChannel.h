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

#ifndef AACE_ALEXA_AUDIO_CHANNEL_H
#define AACE_ALEXA_AUDIO_CHANNEL_H

#include "AACE/Core/PlatformInterface.h"
#include "MediaPlayer.h"
#include "Speaker.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * AudioChannel is the base class for platform interfaces that can play audio. Audio playback control for an AudioChannel
 * is managed by a @c MediaPlayer, and volume and mute control is managed by a @c Speaker.
 *
 * @sa AudioPlayer
 * @sa SpeechSynthesizer
 * @sa Alerts
 * @sa Notifications
 */
class AudioChannel : public aace::core::PlatformInterface {
public:
    AudioChannel( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker, aace::alexa::Speaker::Type speakerType );

    virtual ~AudioChannel() = default;

    /**
     * Returns the @c MediaPlayer instance associated with the AudioChannel
     */
    std::shared_ptr<aace::alexa::MediaPlayer> getMediaPlayer();

    /**
     * Returns the @c Speaker instance associated with the AudioChannel
     */
    std::shared_ptr<aace::alexa::Speaker> getSpeaker();

    /**
     * Returns the @c Speaker::Type associated with the AudioChannel
     */
    aace::alexa::Speaker::Type getSpeakerType();

private:
    std::shared_ptr<aace::alexa::MediaPlayer> m_mediaPlayer;
    std::shared_ptr<aace::alexa::Speaker> m_speaker;
    aace::alexa::Speaker::Type m_speakerType;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_AUDIO_CHANNEL_H
