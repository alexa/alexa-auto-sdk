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
 * The @c NetworkInfoProvider should be extended by the platform implementation to handle network services.
 */
class NetworkInfoProvider : public aace::core::PlatformInterface {
protected:
    NetworkInfoProvider() = default;

public:
    using NetworkStatus = aace::network::NetworkInfoProviderEngineInterface::NetworkStatus;

    virtual ~NetworkInfoProvider() = default;

    /**
     * Called when the Engine needs the current network status.
     *
     * @return @c NetworkStatus of the current network.
     */
    virtual NetworkStatus getNetworkStatus() = 0;

    /**
     * Called when the Engine needs the current signal strength of the network. (RSSI)
     *
     * @return @c int RSSI of the current network.
     */
    virtual int getWifiSignalStrength() = 0;

    /**
     * Tell the Engine that the Wi-Fi network status has changed
     *
     * @param [in] status The connection status of the Wi-Fi network.
     * @param [in] wifiSignalStrength The RSSI of the Wi-Fi connection.
     */
    void networkStatusChanged( NetworkStatus status, int wifiSignalStrength );

    /**
     * @internal
     * Sets engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<NetworkInfoProviderEngineInterface> networkInfoProviderEngineInterface );

private:
    std::shared_ptr<NetworkInfoProviderEngineInterface> m_networkInfoProviderEngineInterface;
};

} // aace::network
} // aace

#endif // AACE_NETWORK_NETWORK_INFO_PROVIDER_H
