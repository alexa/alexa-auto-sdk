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
 * Speaker is the interface for volume and mute control for an @c AudioChannel.
 * Volume and mute settings for the Speaker are independent of each other,
 * and the respective directives from the Engine should not affect the other setting in any way.
 *
 * @sa AudioChannel
 */
abstract public class Speaker extends PlatformInterface
{
    /**
     * Specifies the type of the Speaker
     */
    public enum Type
    {
        /**
         * The Speaker type that is controlled by AVS
         * @hideinitializer
         */
        AVS_SPEAKER("AVS_SPEAKER"),
        /**
         * The Speaker type for Alerts, controlled locally or through Alexa App
         * @hideinitializer
         */
        AVS_ALERTS("AVS_ALERTS");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Type( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public Speaker() {
    }

    /**
     * Notifies the platform implementation to set the absolute volume of the Speaker. The
     * @c volume value should be scaled to fit the needs of the platform.
     *
     * @param  volume The absolute volume to set on the Speaker. @c volume is in the range [0,100].
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean setVolume( byte volume ) {
        return false;
    }

    /**
     * Notifies the platform implementation to make a relative adjustment to the volume setting of the Speaker.
     * The @c delta value is relative to the current volume setting and is positive to
     * increase volume or negative to reduce volume.
     * The volume @c delta value should be scaled to fit the needs of the platform.
     *
     * @param  delta The volume adjustment to apply to the Speaker. @c delta is
     * in the range [-100, 100].
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean adjustVolume( byte delta ) {
        return false;
    }

    /**
     * Notifies the platform implementation to apply a mute setting to the Speaker
     *
     * @param  mute The mute setting to apply to the Speaker. @c true when the Speaker
     * should be muted, @c false when unmuted
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean setMute( boolean mute ) {
        return false;
    }

    /**
     * Returns the current volume setting of the Speaker platform implementation
     *
     * @return The current volume setting of the Speaker platform implementation. The volume returned
     * must be scaled to the range [0,100].
     */
    public byte getVolume() {
        return 0;
    }

    /**
     * Returns the current mute setting of the Speaker platform implementation
     *
     * @return The current mute setting of the Speaker platform implementation.
     * @c true when the Speaker is muted, else @c false
     */
    public boolean isMuted() {
        return false;
    }

    /**
     * Notifies the Engine of a volume change event
     * originating on the platform, such as a user pressing a "volume up" or "volume down"
     * button. If the Speaker is @c Type.AVS_SYNCED, the Engine will respond with a
     * call to @c setVolume() on each AVS-synced Speaker.
     *
     * @param  volume The new volume setting of the Speaker. The @c volume reported
     * must be scaled to the range [0,100].
     *
     * @sa Type
     */
    public void localVolumeSet( byte volume ) {
        localVolumeSet( getNativeObject(), volume );
    }

    /**
     * Notifies the Engine of a mute setting change event
     * originating on the platform, such as a user pressing a "mute" button.
     * If the Speaker is @c Type.AVS_SYNCED, the Engine will respond with a
     * call to @c setMute() on each AVS-synced Speaker.
     *
     * @param  mute The new mute setting of the Speaker. @c true when the Speaker is muted,
     * else @c false
     *
     * @sa Type
     */
    public void localMuteSet( boolean mute ) {
        localMuteSet( getNativeObject(), mute );
    }

    private native void localVolumeSet( long nativeObject, byte volume );
    private native void localMuteSet( long nativeObject, boolean mute );

}

// END OF FILE
