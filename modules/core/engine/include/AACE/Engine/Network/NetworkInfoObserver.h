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

#ifndef AACE_ENGINE_NETWORK_NETWORK_INFO_OBSERVER_H
#define AACE_ENGINE_NETWORK_NETWORK_INFO_OBSERVER_H

#include "AACE/Network/NetworkEngineInterfaces.h"

namespace aace {
namespace engine {
namespace network {

class NetworkInfoObserver {
public:
    using NetworkStatus = aace::network::NetworkInfoProviderEngineInterface::NetworkStatus;

    /**
     * Describes the status of network interface change
     */
    enum class NetworkInterfaceChangeStatus {

        /**
         * The network interface change has begun.
         */
        BEGIN,
        /**
         * Change the network to new provided network interface.
         */
        CHANGE,
        /**
         * The network interface change is completed.
         */
        COMPLETED
    };

    /**
     * Notifies the observer for a change in network status.
     */
    virtual void onNetworkInfoChanged(NetworkStatus status, int wifiSignalStrength) = 0;

    /**
     * Notifies the observer about status of network inteface change.
     */
    virtual void onNetworkInterfaceChangeStatusChanged(
        const std::string& networkInterface,
        NetworkInterfaceChangeStatus status) = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const NetworkInfoObserver::NetworkInterfaceChangeStatus& status) {
    switch (status) {
        case NetworkInfoObserver::NetworkInterfaceChangeStatus::BEGIN:
            stream << "BEGIN";
            break;
        case NetworkInfoObserver::NetworkInterfaceChangeStatus::CHANGE:
            stream << "CHANGE";
            break;
        case NetworkInfoObserver::NetworkInterfaceChangeStatus::COMPLETED:
            stream << "COMPLETED";
            break;
    }
    return stream;
}

}  // namespace network
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_NETWORK_NETWORK_INFO_OBSERVER_H
