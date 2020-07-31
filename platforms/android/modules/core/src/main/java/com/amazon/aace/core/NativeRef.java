/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.core;

abstract public class NativeRef {
    private long mNativeRef = 0;
    private boolean mInitialized = false;

    protected NativeRef() {}

    @Override
    public void finalize() {
        // dispose the native reference when GC is called
        dispose();
    }

    final public void dispose() {
        if (mNativeRef != 0) {
            disposeNativeRef(mNativeRef);
            mNativeRef = 0;
        }
    }

    final public long getNativeRef() {
        if (mInitialized == false) {
            mNativeRef = createNativeRef();
            mInitialized = mNativeRef != 0;
        }
        return mNativeRef;
    }

    final protected void setNativeRef(long nativeRef) throws RuntimeException {
        if (mNativeRef == 0) {
            mNativeRef = nativeRef;
            mInitialized = mNativeRef != 0;
        } else {
            throw new RuntimeException("NativeRef already set");
        }
    }

    // abstract interface methods
    abstract protected long createNativeRef();
    abstract protected void disposeNativeRef(long nativeRef);
}
