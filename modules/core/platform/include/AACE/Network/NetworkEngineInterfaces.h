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

#ifndef AACE_NETWORK_NETWORK_ENGINE_INTERFACES_H
#define AACE_NETWORK_NETWORK_ENGINE_INTERFACES_H

/** @file */

namespace aace {
namespace network {

class NetworkInfoProviderEngineInterface {
public:
    /**
     * The enum NetworkStatus describes the state of network connectivity.
     */
    enum class NetworkStatus {
        /**
         * The network is disconnected.
         */
        DISCONNECTED,
        /**
         * The network is disconnecting.
         */
        DISCONNECTING,
        /**
         * The network is connected.
         */
        CONNECTED,
        /**
         * The network is connecting.
         */
        CONNECTING
    };

    virtual void networkInfoChanged( NetworkStatus status, int wifiSignalStrength ) = 0;
};

} // aace::network
} // aace

#endif // AACE_NETWORK_NETWORK_ENGINE_INTERFACES_H
