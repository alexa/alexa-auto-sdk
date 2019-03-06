/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AASB_CORE_NETWORK_NETWORKINFOPROVIDERHANDLER_H
#define AASB_CORE_NETWORK_NETWORKINFOPROVIDERHANDLER_H

#include <AACE/Network/NetworkInfoProvider.h>

#include "LoggerHandler.h"

/**
 * Describes the status of network connectivity
 * @sa @c aace::network::NetworkInfoProviderEngineInterface::NetworkStatus
 */
using NetworkStatus = aace::network::NetworkInfoProviderEngineInterface::NetworkStatus;

namespace aasb {
namespace core {
namespace network {

class NetworkInfoProviderHandler : public aace::network::NetworkInfoProvider {
public:
    static std::shared_ptr<NetworkInfoProviderHandler> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger);

    /// @name aace::network::NetworkInfoProvider Functions
    /// @{
    NetworkStatus getNetworkStatus() override;
    int getWifiSignalStrength() override;
    /// @}

private:
    NetworkInfoProviderHandler() = default;

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
};

}  // namespace network
}  // namespace core
}  // namespace aasb

#endif  // AASB_CORE_NETWORK_NETWORKINFOPROVIDERHANDLER_H