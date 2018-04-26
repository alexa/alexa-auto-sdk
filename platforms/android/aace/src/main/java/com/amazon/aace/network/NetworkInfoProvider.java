/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/network/NetworkInfoProvider.java
// This is an automatically generated file.

package com.amazon.aace.network;

import com.amazon.aace.core.*;

/**
 * The @c NetworkInfoProvider should be extended by the platform implementation to handle network services.
 */
abstract public class NetworkInfoProvider extends PlatformInterface
{
    /**
     * The enum NetworkStatus describes the state of network connectivity.
     */
    public enum NetworkStatus
    {
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
        private NetworkStatus( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public NetworkInfoProvider() {
    }

    /**
     * Called when the Engine needs the current network status.
     *
     * @return @c NetworkStatus of the current network.
     */
    public NetworkStatus getNetworkStatus() {
        return null;
    }

    /**
     * Called when the Engine needs the current signal strength of the network. (RSSI)
     *
     * @return @c int RSSI of the current network.
     */
    public int getWifiSignalStrength() {
        return 0;
    }

    /**
     * Tell the Engine that the Wi-Fi network status has changed
     *
     * @param [in] status The connection status of the Wi-Fi network.
     * @param [in] wifiSignalStrength The RSSI of the Wi-Fi connection.
     */
    public void networkStatusChanged( NetworkStatus status, int wifiSignalStrength ) {
        networkStatusChanged( getNativeObject(), status, wifiSignalStrength );
    }

    private native void networkStatusChanged( long nativeObject, NetworkStatus status, int wifiSignalStrength );
}

// END OF FILE
