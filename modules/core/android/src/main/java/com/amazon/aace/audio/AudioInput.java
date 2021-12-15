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

abstract public class AudioInput extends NativeRef {
    public boolean startAudioInput() {
        return false;
    }

    public boolean stopAudioInput() {
        return false;
    }

    final public long write(byte[] data) {
        return write(getNativeRef(), data, 0, data.length);
    }

    final public long write(byte[] data, long size) {
        return write(getNativeRef(), data, 0, size);
    }

    final public long write(byte[] data, long offset, long size) {
        return write(getNativeRef(), data, offset, size);
    }

    protected long createNativeRef() {
        return createBinder();
    }

    protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native long write(long nativeObject, byte[] data, long offset, long size);
}
