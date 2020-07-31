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

#ifndef AACE_NETWORK_NETWORK_INFO_PROVIDER_H
#define AACE_NETWORK_NETWORK_INFO_PROVIDER_H

#include <string>
#include <chrono>

#include "AACE/Core/PlatformInterface.h"
#include "NetworkEngineInterfaces.h"

/** @file */

namespace aace {
namespace network {

/**
 * NetworkInfoProvider should be extended to report network connectivity events to the Engine.
 */
class NetworkInfoProvider : public aace::core::PlatformInterface {
protected:
    NetworkInfoProvider() = default;

public:
    /**
     * Describes the status of network connectivity
     * @sa @c aace::network::NetworkInfoProviderEngineInterface::NetworkStatus
     */
    using NetworkStatus = aace::network::NetworkInfoProviderEngineInterface::NetworkStatus;

    virtual ~NetworkInfoProvider();

    /**
     * Returns the current network connectivity status on the platform
     *
     * @return The connectivity status of the current network
     */
    virtual NetworkStatus getNetworkStatus() = 0;

    /**
     * Returns the current signal strength (RSSI) of the WiFi connection on the platform
     *
     * @return The RSSI of the WiFi connection
     */
    virtual int getWifiSignalStrength() = 0;

    /**
     * Notifies the Engine of a WiFi network status change on the platform
     *
     * @param [in] status The connection status of the WiFi network
     * @param [in] wifiSignalStrength The RSSI of the WiFi connection
     */
    void networkStatusChanged(NetworkStatus status, int wifiSignalStrength);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<NetworkInfoProviderEngineInterface> networkInfoProviderEngineInterface);

private:
    std::shared_ptr<NetworkInfoProviderEngineInterface> m_networkInfoProviderEngineInterface;
};

}  // namespace network
}  // namespace aace

#endif  // AACE_NETWORK_NETWORK_INFO_PROVIDER_H
