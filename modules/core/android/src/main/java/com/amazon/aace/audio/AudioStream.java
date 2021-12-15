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

import com.amazon.aace.core.NativeRef;

final public class AudioStream extends NativeRef {
    /**
     * Describes the playback state of the platform media player
     */
    public enum Encoding {
        /**
         * @hideinitializer
         */
        UNKNOWN("UNKNOWN"),
        /**
         * @hideinitializer
         */
        LPCM("LPCM"),
        /**
         * @hideinitializer
         */
        MP3("MP3"),
        /**
         * @hideinitializer
         */
        OPUS("OPUS");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Encoding(String name) {
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
     * Key/value pair used for audio stream meta-data.
     */
    public static class AudioStreamProperty {
        /// The property key
        private String mKey;
        /// The property value
        private String mValue;

        /**
         * AudioStreamProperty constructor
         *
         * @param  key The property key
         * @param  value The property value
         */
        public AudioStreamProperty(String key, String value) {
            mKey = key;
            mValue = value;
        }

        /**
         * @return The property key
         */
        public String getKey() {
            return mKey;
        }

        /**
         * @return The property value
         */
        public String getValue() {
            return mValue;
        }
    }

    /**
     * Reads audio data from the @c AudioStream. Audio data will be available while @c isClosed() returns false.
     *
     * @param  data The buffer where audio data should be copied
     * @param  size The size of the buffer
     * @return The number of bytes read, 0 if the end of stream is reached or data is not currently available,
     * or -1 if an error occurred
     */
    final public int read(byte[] buffer) {
        return read(getNativeRef(), buffer, 0, buffer.length);
    }

    /**
     * Reads audio data from the @c AudioStream. Audio data will be available while @c isClosed() returns false.
     *
     * @param  data The buffer where audio data should be copied
     * @param  size The size of the buffer
     * @return The number of bytes read, 0 if the end of stream is reached or data is not currently available,
     * or -1 if an error occurred
     */
    final public int read(byte[] data, int offset, int size) {
        return read(getNativeRef(), data, offset, size);
    }

    /**
     * @return @c true if the @c AudioStream is closed and no more data is available to read.
     */
    final public boolean isClosed() {
        return isClosed(getNativeRef());
    }

    /**
     * @return The @c Encoding type of the data returned by this @c AudioStream.
     */
    final public Encoding getEncoding() {
        return getEncoding(getNativeRef());
    }

    /**
     * @return The @c AudioFormat for this @c AudioStream.
     */
    final public AudioFormat getAudioFormat() {
        return getAudioFormat(getNativeRef());
    }

    /**
     * @return List of meta-data properties for this @c AudioStream.
     */
    final public AudioStreamProperty[] getProperties() {
        return getProperties(getNativeRef());
    }

    protected long createNativeRef() {
        return 0;
    }

    protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native void disposeBinder(long nativeRef);
    private native int read(long nativeObject, byte[] data, long offset, long size);
    private native boolean isClosed(long nativeObject);
    private native Encoding getEncoding(long nativeObject);
    private native AudioFormat getAudioFormat(long nativeObject);
    private native AudioStreamProperty[] getProperties(long nativeObject);
}
