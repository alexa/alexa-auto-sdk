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

package com.amazon.aace.audio;

import com.amazon.aace.core.PlatformInterface;

/**
 * AudioOutputProvider should be extended to provide an audio output source to the Engine.
 */
abstract public class AudioOutputProvider extends PlatformInterface {
    /**
     * Specifies the audio input type for an input channel
     */
    public enum AudioOutputType {
        TTS("TTS"),
        MUSIC("MUSIC"),
        NOTIFICATION("NOTIFICATION"),
        ALARM("ALARM"),
        EARCON("EARCON"),
        COMMUNICATION("COMMUNICATION"),
        RINGTONE("RINGTONE");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        AudioOutputType(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }

        /**
         * @internal
         */
        public char toChar() {
            return m_name.charAt(0);
        }
    }

    abstract public AudioOutput openChannel(String name, AudioOutputType type);

    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
}
