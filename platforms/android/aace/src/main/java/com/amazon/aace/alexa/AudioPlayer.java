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

// aace/alexa/AudioPlayer.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

/**
 * AudioPlayer should be extended to handle audio output from the Engine.
 * The AudioPlayer @c MediaPlayer and @c Speaker will receive directives from the Engine to handle audio playback.
 *
 * @note Audio playback control operations such as on-platform button presses must be routed through
 * the @c PlaybackController.
 *
 * @sa AudioChannel
 *
 * @sa PlaybackController
 *
 * @sa TemplateRuntime::renderPlayerInfo()
 */
public class AudioPlayer extends AudioChannel
{
    /**
     * AudioPlayer should be extended to handle audio output from the Engine.
     * The AudioPlayer @c MediaPlayer and @c Speaker will receive directives from the Engine to handle audio playback.
     *
     * @note Audio playback control operations such as on-platform button presses must be routed through
     * the @c PlaybackController.
     *
     * @sa AudioChannel
     *
     * @sa PlaybackController
     *
     * @sa TemplateRuntime::renderPlayerInfo()
     */
    public AudioPlayer( MediaPlayer mediaPlayer, Speaker speaker ) {
        super( mediaPlayer, speaker, Speaker.Type.AVS_SYNCED );
    }

}

// END OF FILE
