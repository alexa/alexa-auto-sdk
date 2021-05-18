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

import com.amazon.aace.core.NativeRef;

abstract public class GATTServer extends NativeRef {
    /**
     * Describes the state of connection to a calling device
     *
     * @sa PhoneCallController::connectionStateChanged
     */
    public enum ConnectionState {
        CONNECTED("CONNECTED"),
        DISCONNECTED("DISCONNECTED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ConnectionState(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public GATTServer() {}

    public boolean start(String configuration) {
        return false;
    }

    public boolean stop() {
        return false;
    }

    public boolean setCharacteristicValue(String serviceId, String characteristicId, byte[] data) {
        return false;
    }

    final public void connectionStateChanged(String device, ConnectionState state) {
        connectionStateChanged(getNativeRef(), device, state);
    }

    final public boolean requestCharacteristic(
            String device, int requestId, String serviceId, String characteristicId, byte[] data) {
        return requestCharacteristic(getNativeRef(), device, requestId, serviceId, characteristicId, data);
    }

    final public boolean requestDescriptor(
            String device, int requestId, String serviceId, String characteristicId, String descriptorId, byte[] data) {
        return requestDescriptor(getNativeRef(), device, requestId, serviceId, characteristicId, descriptorId, data);
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
    private native void connectionStateChanged(long nativeRef, String device, ConnectionState state);
    private native boolean requestCharacteristic(
            long nativeRef, String device, int requestId, String serviceId, String characteristicId, byte[] data);
    private native boolean requestDescriptor(long nativeRef, String device, int requestId, String serviceId,
            String characteristicId, String descriptorId, byte[] data);
}

// END OF FILE
