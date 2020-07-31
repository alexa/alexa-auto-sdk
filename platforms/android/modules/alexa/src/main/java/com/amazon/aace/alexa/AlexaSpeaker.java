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
 * AlexaSpeaker is the interface for setting the Alexa volume and mute controls.
 */
abstract public class AlexaSpeaker extends PlatformInterface {
    /**
     * Describes the state of Alexa dialog interaction
     */
    public enum SpeakerType {
        /**
         * The Speaker type that is controlled locally by the platform
         * @hideinitializer
         */
        ALEXA_VOLUME("ALEXA_VOLUME"),

        /**
         * The Speaker type that is controlled locally by the platform
         * @hideinitializer
         */
        ALERTS_VOLUME("ALERTS_VOLUME");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private SpeakerType(String name) {
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
     * Notifies the platform implementation that the speaker settings have changed for
     * a specific speaker type.
     *
     * @note Calling @c AlexaSpeaker::localSetVolume,
     * @c AlexaSpeaker::localAdjustVolume, or @c AlexaSpeaker::localSetMute from inside this
     * function will cause deadlock.
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] local @c true if the change originated from calling @c AlexaSpeaker::localSetVolume,
     * @c AlexaSpeaker::localAdjustVolume, or @c AlexaSpeaker::localSetMute.
     * @param [in] volume The new volume setting of the Speaker. The @c volume reported
     * will be scaled to the range [0,100].
     * @param [in] mute The mute setting of the Speaker. @c true when the Speaker is muted,
     * else @c false.
     */
    public void speakerSettingsChanged(SpeakerType type, boolean local, byte volume, boolean mute) {}

    /**
     * Notifies the Engine of a volume change event
     * originating on the platform, such as a user pressing a "volume up" or "volume down"
     * button. If the Speaker is @c Type::AVS_SPEAKER_VOLUME, the Engine will respond with a
     * call to @c setVolume() on each AVS-synced Speaker.
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] volume The new volume setting of the Speaker. The @c volume reported
     * must be scaled to the range [0,100].
     */
    public final void localSetVolume(SpeakerType type, byte volume) {
        localSetVolume(getNativeRef(), type, volume);
    }

    /**
     * Notifies the Engine of a relative adjustment to the volume setting of the Speaker,
     * originating on the platform.
     * The @c delta value is relative to the current volume setting and is positive to
     * increase volume or negative to reduce volume.
     * The volume @c delta value should be scaled to fit the needs of the platform.
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] delta The volume adjustment to apply to the Speaker. @c delta is
     * in the range [-100, 100].
     */
    public final void localAdjustVolume(SpeakerType type, byte delta) {
        localAdjustVolume(getNativeRef(), type, delta);
    }

    /**
     * Notifies the Engine of a mute setting change event
     * originating on the platform, such as a user pressing a "mute" button.
     * If the Speaker is @c Type::AVS_SPEAKER_VOLUME, the Engine will respond with a
     * call to @c setMute() on each AVS-synced Speaker.
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] mute The new mute setting of the Speaker. @c true when the Speaker is muted,
     * else @c false
     */
    public final void localSetMute(SpeakerType type, boolean mute) {
        localSetMute(getNativeRef(), type, mute);
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
    private native void localSetVolume(long nativeRef, SpeakerType type, byte volume);
    private native void localAdjustVolume(long nativeRef, SpeakerType type, byte delta);
    private native void localSetMute(long nativeRef, SpeakerType type, boolean mute);
}
