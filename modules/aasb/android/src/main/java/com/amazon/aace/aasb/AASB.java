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

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c AASB class should be extended by to provide an implementation of the AASB interface.
 */
abstract public class AASB extends PlatformInterface {
    public void messageReceived(String message) {}

    public final void publish(String message) {
        publish(getNativeRef(), message);
    }

    public final AASBStream openStream(String streamId, AASBStream.Mode mode) {
        return openStream(getNativeRef(), streamId, mode);
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
    private native void publish(long nativeRef, String message);
    private native AASBStream openStream(long nativeRef, String streamId, AASBStream.Mode mode);
}

// END OF FILE
