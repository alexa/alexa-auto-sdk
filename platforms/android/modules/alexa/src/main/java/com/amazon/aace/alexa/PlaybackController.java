/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 * PlaybackController should be extended to handle playback control
 * operations such as on-platform button presses for the @c AudioPlayer. The Engine will respond to PlaybackController
 * events with playback control directives on the @c AudioPlayer @c MediaPlayer.
 *
 * @note The TemplateRuntime.renderPlayerInfo() payload includes information about what
 * playback control buttons should be enabled on screen with a player info display card.
 *
 * @sa AudioPlayer
 */
abstract public class PlaybackController extends PlatformInterface {
    public PlaybackController() {}

    /**
     * Describes the playback controller button types
     */
    public enum PlaybackButton {
        /**
         * 'Play' button.
         * @hideinitializer
         */
        PLAY("PLAY"),
        /**
         * 'Pause' button.
         * @hideinitializer
         */
        PAUSE("PAUSE"),
        /**
         * 'Next' button.
         * @hideinitializer
         */
        NEXT("NEXT"),
        /**
         * 'Previous' button.
         * @hideinitializer
         */
        PREVIOUS("PREVIOUS"),
        /**
         * 'Skip Forward' button.
         * @hideinitializer
         */
        SKIP_FORWARD("SKIP_FORWARD"),
        /**
         * 'Skip Backward' button.
         * @hideinitializer
         */
        SKIP_BACKWARD("SKIP_BACKWARD");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private PlaybackButton(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Describes the playback controller toggle types
     */
    public enum PlaybackToggle {
        /**
         * 'Shuffle' toggle.
         * @hideinitializer
         */
        SHUFFLE("SHUFFLE"),
        /**
         * 'Loop' toggle.
         * @hideinitializer
         */
        LOOP("LOOP"),
        /**
         * 'Repeat' toggle.
         * @hideinitializer
         */
        REPEAT("REPEAT"),
        /**
         * 'Thumbs Up' toggle.
         * @hideinitializer
         */
        THUMBS_UP("THUMBS_UP"),
        /**
         * 'Thumbs Down' toggle.
         * @hideinitializer
         */
        THUMBS_DOWN("THUMBS_DOWN");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private PlaybackToggle(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Notifies the Engine of a platform button request (i.e. Play/Pause/Next/Previous/Skip Forward/Skip Backward).
     * For certain playback types, the Engine will issue playback directives to the @c AudioPlayer @c MediaPlayer
     * to control playback on the platform.
     * @param  button The playback button type
     */
    public final void buttonPressed(PlaybackButton button) {
        buttonPressed(getNativeRef(), button);
    }

    /**
     * Notifies the Engine of a platform toggle request (i.e. Shuffle/Loop/Repeat/Thumbs Up/Thumbs Down).
     * For certain playback types, the Engine will issue playback directives to the @c AudioPlayer @c MediaPlayer
     * to control playback on the platform.
     * @param  toggle The playback toggle type
     * @param  action The playback toggle action
     */
    public final void togglePressed(PlaybackToggle toggle, boolean action) {
        togglePressed(getNativeRef(), toggle, action);
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native void buttonPressed(long nativeObject, PlaybackButton button);
    private native void togglePressed(long nativeObject, PlaybackToggle toggle, boolean action);
}
