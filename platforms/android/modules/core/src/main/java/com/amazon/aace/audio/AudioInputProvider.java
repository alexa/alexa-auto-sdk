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
 * AudioInputProvider should be extended to provide an audio input source to the Engine.
 */
abstract public class AudioInputProvider extends PlatformInterface {
    /**
     * Specifies the audio input type for an input channel
     */
    public enum AudioInputType {
        VOICE("VOICE"),
        COMMUNICATION("COMMUNICATION"),
        LOOPBACK("LOOPBACK");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        AudioInputType(String name) {
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

    abstract public AudioInput openChannel(String name, AudioInputType type);

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
