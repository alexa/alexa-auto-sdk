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

package com.amazon.aace.aasb;

import com.amazon.aace.core.NativeRef;

public class AASBStream extends NativeRef {
    /// An enumeration representing the stream operation mode.
    public enum Mode {
        /**
         * @hideinitializer
         */
        READ("READ"),
        /**
         * @hideinitializer
         */
        WRITE("WRITE"),
        /**
         * @hideinitializer
         */
        READ_WRITE("READ_WRITE");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Mode(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }
    ;

    /**
     * Reads audio data from the @c AASBStream. Audio data will be available while @c isClosed() returns false.
     *
     * @param  data The buffer where audio data should be copied
     * @return The number of bytes read, 0 if the end of stream is reached or data is not currently available,
     * or -1 if an error occurred
     */
    final public int read(byte[] data) {
        return read(getNativeRef(), data, 0, data.length);
    }

    /**
     * Reads audio data from the @c AASBStream. Audio data will be available while @c isClosed() returns false.
     *
     * @param  data The buffer where audio data should be copied
     * @param  offset The offset in the buffer where audio data should be copied
     * @param  size The size of the buffer
     * @return The number of bytes read, 0 if the end of stream is reached or data is not currently available,
     * or -1 if an error occurred
     */
    final public int read(byte[] data, int offset, int size) {
        return read(getNativeRef(), data, offset, size);
    }

    /**
     * Writes data to the @c AASBStream.
     *
     * @param data The data to be written to the stream
     * @return The number of bytes successfully written to the stream or a negative error code
     * if data could not be written
     */
    final public int write(byte[] data) {
        return write(getNativeRef(), data, 0, data.length);
    }

    /**
     * Writes data to the @c AASBStream.
     *
     * @param data The data to be written to the stream
     * @param offset The offset of the data to be written to the stream
     * @param size The number of bytes to be written
     * @return The number of bytes successfully written to the stream or a negative error code
     * if data could not be written
     */
    final public int write(byte[] data, int offset, int size) {
        return write(getNativeRef(), data, offset, size);
    }

    /**
     * @return @c true if the @c AASBStream is closed and no more data is available to read.
     */
    final public boolean isClosed() {
        return isClosed(getNativeRef());
    }

    /**
     * @return permitted stream operation mode.
     */
    final public Mode getMode() {
        return getMode(getNativeRef());
    }

    // NativeRef implementation
    protected long createNativeRef() {
        return 0;
    }

    protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native void disposeBinder(long nativeRef);
    private native int read(long nativeObject, byte[] data, long offset, long size);
    private native int write(long nativeObject, byte[] data, long offset, long size);
    private native boolean isClosed(long nativeObject);
    private native Mode getMode(long nativeObject);
}

// END OF FILE
