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

#ifndef AACE_NETWORK_NETWORK_ENGINE_INTERFACES_H
#define AACE_NETWORK_NETWORK_ENGINE_INTERFACES_H

/** @file */

#include <iostream>

namespace aace {
namespace network {

/**
 * NetworkInfoProviderEngineInterface
 */
class NetworkInfoProviderEngineInterface {
public:
    /**
     * Describes the status of network connectivity
     */
    enum class NetworkStatus {

        /**
         * The network is unknown.
         */
        UNKNOWN,
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

    virtual void networkInfoChanged(NetworkStatus status, int wifiSignalStrength) = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const NetworkInfoProviderEngineInterface::NetworkStatus& status) {
    switch (status) {
        case NetworkInfoProviderEngineInterface::NetworkStatus::UNKNOWN:
            stream << "UNKNOWN";
            break;
        case NetworkInfoProviderEngineInterface::NetworkStatus::DISCONNECTED:
            stream << "DISCONNECTED";
            break;
        case NetworkInfoProviderEngineInterface::NetworkStatus::DISCONNECTING:
            stream << "DISCONNECTING";
            break;
        case NetworkInfoProviderEngineInterface::NetworkStatus::CONNECTED:
            stream << "CONNECTED";
            break;
        case NetworkInfoProviderEngineInterface::NetworkStatus::CONNECTING:
            stream << "CONNECTING";
            break;
    }
    return stream;
}

}  // namespace network
}  // namespace aace

#endif  // AACE_NETWORK_NETWORK_ENGINE_INTERFACES_H
