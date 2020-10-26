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
 * TemplateRuntime should be extended to handle directives from the Engine for
 * rendering visual metadata with display cards.
 * Metadata is provided as structured JSON and should be parsed by the platform implementation
 * for rendering cards that adhere to design guidelines
 * for the platform device type. For screen-specific design guidance, see the AVS UX Design Overview:
 * https://developer.amazon.com/docs/alexa-voice-service/ux-design-overview.html#displaycards
 */
abstract public class TemplateRuntime extends PlatformInterface {
    /**
     * An enum class used to specify the levels of focus that a Channel can have.
     */
    public enum FocusState {
        /**
         * Represents the highest focus a Channel can have.
         * @hideinitializer
         */
        FOREGROUND("FOREGROUND"),

        /**
         * Represents the intermediate level focus a Channel can have.
         * @hideinitializer
         */
        BACKGROUND("BACKGROUND"),

        /**
         * This focus is used to represent when a Channel is not being used.
         * @hideinitializer
         */
        NONE("NONE");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private FocusState(String name) {
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
     * An enum class used to specify audio player state for TemplateRuntime.
     */
    public enum PlayerActivity {
        /**
         * Audio playback has not yet begun.
         * @hideinitializer
         */
        IDLE("IDLE"),

        /**
         * Audio is currently playing.
         * @hideinitializer
         */
        PLAYING("PLAYING"),

        /**
         * Audio playback is stopped, either from a stop directive or playback error.
         * @hideinitializer
         */
        STOPPED("STOPPED"),

        /**
         * Audio playback is paused.
         * @hideinitializer
         */
        PAUSED("PAUSED"),

        /**
         * Audio playback is stalled because a buffer underrun has occurred.
         * @hideinitializer
         */
        BUFFER_UNDERRUN("BUFFER_UNDERRUN"),

        /**
         * Audio playback is finished.
         * @hideinitializer
         */
        FINISHED("FINISHED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private PlayerActivity(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public TemplateRuntime() {}

    /**
     * Provides visual metadata associated with a user request to Alexa.
     * The platform implementation should parse the template metadata and render
     * a display card for the user.
     *
     * For supported templates and rendering guidelines, see https://alexa.design/DevDocRenderTemplate
     *
     * @param [in] payload Renderable template metadata in structured JSON format
     * @param [in] focusState The @c FocusState of the channel used by TemplateRuntime interface
     */
    public void renderTemplate(String payload, FocusState focusState) {}

    /**
     * Provides visual metadata associated with a user request to Alexa for audio playback.
     * The platform implementation should parse the player info metadata and render a display
     * card for the user. The audioPlayerState and offset are useful for implementing the
     * progress bar in the display card. It is assumed that the client is responsible
     * for progressing the progress bar when the @c AudioPlayer is in PLAYING state.
     *
     * For rendering guidelines, see https://amzn.to/DevDocTemplatePlayerInfo
     *
     * @param [in] payload Renderable player info metadata in structured JSON format
     * @param [in] audioPlayerState The state of the @c AudioPlayer
     * @param [in] offset The offset in millisecond of the media that @c AudioPlayer is handling
     * @param [in] focusState The @c FocusState of the channel used by TemplateRuntime interface
     * @sa PlaybackController
     * @sa AudioPlayer
     */
    public void renderPlayerInfo(String payload, PlayerActivity audioPlayerState, long offset, FocusState focusState) {}

    /**
     * Notifies the platform implementation to dismiss the template display card
     */
    public void clearTemplate() {}

    /**
     * Notifies the platform implementation to dismiss the player info display card
     */
    public void clearPlayerInfo() {}

    /**
     * Notifies the Engine that a display card has been cleared from the screen. Upon getting
     * this notification, the @c TemplateRuntime will release the visual channel.
     */
    public final void displayCardCleared() {
        displayCardCleared(getNativeRef());
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
    private native void displayCardCleared(long nativeRef);
}
