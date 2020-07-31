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

package com.amazon.aace.network;

import com.amazon.aace.core.*;

/**
 * NetworkInfoProvider should be extended to report network connectivity events to the Engine.
 */
abstract public class NetworkInfoProvider extends PlatformInterface {
    /**
     * Describes the status of network connectivity
     */
    public enum NetworkStatus {
        /**
         * The network is unknown.
         * @hideinitializer
         */
        UNKNOWN("UNKNOWN"),
        /**
         * The network is disconnected.
         * @hideinitializer
         */
        DISCONNECTED("DISCONNECTED"),
        /**
         * The network is disconnecting.
         * @hideinitializer
         */
        DISCONNECTING("DISCONNECTING"),
        /**
         * The network is connected.
         * @hideinitializer
         */
        CONNECTED("CONNECTED"),
        /**
         * The network is connecting.
         * @hideinitializer
         */
        CONNECTING("CONNECTING");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private NetworkStatus(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public NetworkInfoProvider() {}

    /**
     * Returns the current network connectivity status on the platform
     *
     * @return The connectivity status of the current network
     */
    public NetworkStatus getNetworkStatus() {
        return null;
    }

    /**
     * Returns the current signal strength (RSSI) of the WiFi connection on the platform
     *
     * @return The RSSI of the WiFi connection
     */
    public int getWifiSignalStrength() {
        return 0;
    }

    /**
     * Notifies the Engine of a WiFi network status change on the platform
     *
     * @param  status The connection status of the WiFi network
     *
     * @param  wifiSignalStrength The RSSI of the WiFi connection
     */
    public void networkStatusChanged(NetworkStatus status, int wifiSignalStrength) {
        networkStatusChanged(getNativeRef(), status, wifiSignalStrength);
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
    private native void networkStatusChanged(long nativeRef, NetworkStatus status, int wifiSignalStrength);
}

// END OF FILE
