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

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * AudioChannel is the base class for platform interfaces that can play audio. Audio playback control for an AudioChannel
 * is managed by a @c MediaPlayer, and volume and mute control is managed by a @c Speaker.
 *
 * @sa AudioPlayer
 *
 * @sa SpeechSynthesizer
 *
 * @sa Alerts
 *
 * @sa Notifications
 */
abstract public class AudioChannel extends PlatformInterface
{
    private MediaPlayer m_mediaPlayer;
    private Speaker m_speaker;
    private Speaker.Type m_type;

    public AudioChannel( MediaPlayer mediaPlayer, Speaker speaker, Speaker.Type type ) {
        m_mediaPlayer = mediaPlayer;
        m_speaker = speaker;
        m_type = type;
    }

    /**
     * Returns the @c MediaPlayer instance associated with the AudioChannel
     */
    public MediaPlayer getMediaPlayer() {
        return m_mediaPlayer;
    }

    /**
     * Returns the @c Speaker instance associated with the AudioChannel
     */
    public Speaker getSpeaker() {
        return m_speaker;
    }

    /**
     * Returns the @c Speaker.Type associated with the AudioChannel
     */
    public Speaker.Type getType() {
        return m_type;
    }
}

// END OF FILE
