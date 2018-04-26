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

// aace/alexa/Speaker.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c Speaker class is the base class for platform interfaces that have volume control.
 */
abstract public class Speaker extends PlatformInterface
{
    /**
     * This enum provides the type of the @c Speaker class.
     */
    public enum Type
    {
        /**
         * Speaker source that should be synced with AVS.
         * @hideinitializer
         */
        AVS_SYNCED("AVS_SYNCED"),
        /**
         * Speaker source that will not be synced with AVS.
         * @hideinitializer
         */
        LOCAL("LOCAL");

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
     * Called when the platform implementation should change the volume.
     *
     * @param [in] volume The absolute volume level scaled from 0 (min) to 100 (max).
     * @return @c true if the call was handled successfully.
     */
    public boolean setVolume( byte volume ) {
        return false;
    }

    /**
     * Called when the platform implementation should adjust the volume to a relative level.
     *
     * @param [in] delta The relative volume adjustment. A positive or negative value used to increase or decrease volume in relation to the current volume setting.
     * @return @c true if the call was handled successfully.
     */
    public boolean adjustVolume( byte delta ) {
        return false;
    }

    /**
     * Called when the platform implementation should mute/unmute.
     *
     * @param [in] mute @c true when the media player is muted, and @c false when unmuted.
     * @return @c true if the call was handled successfully.
     */
    public boolean setMute( boolean mute ) {
        return false;
    }

    /**
     * Called when the Engine needs the platform's current volume.
     */
    public byte getVolume() {
        return 0;
    }

    /**
     * Called when the Engine needs the platform's muting state.
     */
    public boolean isMuted() {
        return false;
    }
}

// END OF FILE
