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

package com.amazon.aace.bluetooth;

import com.amazon.aace.core.PlatformInterface;

abstract public class BluetoothProvider extends PlatformInterface {
    public BluetoothProvider() {}

    public GATTServer createGATTServer() {
        return null;
    }

    public BluetoothServerSocket listenUsingRfcomm(String name, String uuid) {
        return null;
    }

    public BluetoothServerSocket listenUsingiAP2(String protocol) {
        return null;
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
}

// END OF FILE
