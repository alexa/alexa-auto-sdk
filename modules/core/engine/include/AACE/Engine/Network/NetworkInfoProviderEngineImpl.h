/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/Metrics/DurationDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricRecorderServiceInterface.h>
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
    NetworkInfoProviderEngineImpl(
        const std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface>& metricRecorder);

public:
    static std::shared_ptr<NetworkInfoProviderEngineImpl> create(
        const std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface>& metricRecorder);

    // aace::engine::network::NetworkObservableInterface
    void addObserver(std::shared_ptr<NetworkInfoObserver> observer) override;
    void removeObserver(std::shared_ptr<NetworkInfoObserver> observer) override;

    // NetworkInfoProviderEngineInterface
    virtual void networkInfoChanged(NetworkStatus status, int wifiSignalStrength) override;

    bool setNetworkInterface(const std::string& networkInterface);
    bool setNetworkHttpProxyHeader(const std::string& headers);

    /**
     * Handle any operations required when the Engine is stopped.
     */
    void stop();

private:
    /**
     * Set of network info observers. Access is serialized by @c m_mutex.
     */
    std::unordered_set<std::shared_ptr<NetworkInfoObserver>> m_observers;

    /**
     * Whether there is an internet connection. Access is serialized by
     * @c m_mutex.
    */
    bool m_isConnected;

    /// Mutex to serialize access to member variables.
    std::mutex m_mutex;

    /// The metric recorder.
    std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> m_metricRecorder;

    /// Builder for network connection time metric.
    aace::engine::metrics::DurationDataPointBuilder m_connectedDuration;

    /// Builder for network disconnection time metric.
    aace::engine::metrics::DurationDataPointBuilder m_disconnectedDuration;
};

}  // namespace network
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_NETWORK_NETWORK_INFO_PROVIDER_ENGINE_IMPL_H
