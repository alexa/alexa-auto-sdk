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

// aace/alexa/PlaybackController.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c PlaybackController class should be extended by the platform implementation to handle media controller events occurring on the platform.
 */
abstract public class PlaybackController extends PlatformInterface
{
    public PlaybackController() {
    }

    /**
     * Notify the Engine that the Play button has been pressed.
     */
    public void playButtonPressed() {
        playButtonPressed( getNativeObject() );
    }

    /**
     * Notify the Engine that the Pause button has been pressed.
     */
    public void pauseButtonPressed() {
        pauseButtonPressed( getNativeObject() );
    }

    /**
     * Notify the Engine that the Next button has been pressed.
     */
    public void nextButtonPressed() {
        nextButtonPressed( getNativeObject() );
    }

    /**
     * Notify the Engine that the Previous button has been pressed.
     */
    public void previousButtonPressed() {
        previousButtonPressed( getNativeObject() );
    }

    private native void playButtonPressed( long nativeObject );
    private native void pauseButtonPressed( long nativeObject );
    private native void nextButtonPressed( long nativeObject );
    private native void previousButtonPressed( long nativeObject );
}

// END OF FILE
