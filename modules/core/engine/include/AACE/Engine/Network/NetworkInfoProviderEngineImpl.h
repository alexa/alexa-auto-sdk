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

#ifndef AACE_ENGINE_NETWORK_NETWORK_INFO_PROVIDER_ENGINE_IMPL_H
#define AACE_ENGINE_NETWORK_NETWORK_INFO_PROVIDER_ENGINE_IMPL_H

#include <unordered_set>
#include <mutex>
#include <memory>

#include "AACE/Engine/Network/NetworkObservableInterface.h"
#include "AACE/Network/NetworkEngineInterfaces.h"

#include "NetworkInfoObserver.h"

namespace aace {
namespace engine {
namespace network {

class NetworkInfoProviderEngineImpl
        : public aace::network::NetworkInfoProviderEngineInterface
        , public NetworkObservableInterface {
private:
    NetworkInfoProviderEngineImpl() = default;

public:
    static std::shared_ptr<NetworkInfoProviderEngineImpl> create();

    // aace::engine::network::NetworkObservableInterface
    void addObserver(std::shared_ptr<NetworkInfoObserver> observer) override;
    void removeObserver(std::shared_ptr<NetworkInfoObserver> observer) override;

    // NetworkInfoProviderEngineInterface
    virtual void networkInfoChanged(NetworkStatus status, int wifiSignalStrength) override;

    bool setNetworkInterface(const std::string& networkInterface);

private:
    std::unordered_set<std::shared_ptr<NetworkInfoObserver>> m_observers;
    std::mutex m_mutex;
};

}  // namespace network
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_NETWORK_NETWORK_INFO_PROVIDER_ENGINE_IMPL_H
