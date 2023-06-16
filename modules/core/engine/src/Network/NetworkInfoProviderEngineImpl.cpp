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

#include <stdexcept>
#include <sstream>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/CounterDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>
#include <AACE/Engine/Metrics/StringDataPointBuilder.h>
#include <AACE/Engine/Network/NetworkInfoProviderEngineImpl.h>

namespace aace {
namespace engine {
namespace network {

using namespace aace::engine::metrics;

/// Source name for network connection status metrics
static const std::string METRIC_SOURCE = "NetworkConnectionStatus";

// String to identify log entries originating from this file.
static const std::string TAG("aace.core.NetworkInfoProviderEngineImpl");

/// Network connection change count metric key
static const std::string METRIC_CONNECTION_CHANGE_COUNT = "ConnectionStatusChangeCount";

/// Network connection status duration metric key
static const std::string METRIC_CONNECTION_STATUS_DURATION = "StatusDuration";

/// Network connection status type metric dimension key
static const std::string METRIC_CONNECTION_STATUS_TYPE = "StatusType";

/// Network disconnected metric type
static const std::string METRIC_STATUS_DISCONNECTED = "Disconnected";

/// Network connected metric type
static const std::string METRIC_STATUS_CONNECTED = "Connected";

/**
 * Records a metric with the specified data points.
 */
static void submitMetric(
    const std::shared_ptr<MetricRecorderServiceInterface>& recorder,
    const std::vector<DataPoint>& dataPoints) {
    auto metricBuilder =
        MetricEventBuilder().withSourceName(METRIC_SOURCE).withNoneAgentId().withBufferType(BufferType::SKIP_BUFFER);
    metricBuilder.addDataPoints(dataPoints);
    try {
        recordMetric(recorder, metricBuilder.build());
    } catch (std::invalid_argument& ex) {
        AACE_ERROR(LX(TAG).m("Failed to record metric").d("reason", ex.what()));
    }
}

NetworkInfoProviderEngineImpl::NetworkInfoProviderEngineImpl(
    const std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface>& metricRecorder) :
        m_metricRecorder{metricRecorder} {
    m_disconnectedDuration.startTimer();
}

std::shared_ptr<NetworkInfoProviderEngineImpl> NetworkInfoProviderEngineImpl::create(
    const std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface>& metricRecorder) {
    try {
        ThrowIfNull(metricRecorder, "metricRecorderIsNull");
        auto networkInfoProviderEngineImpl =
            std::shared_ptr<NetworkInfoProviderEngineImpl>(new NetworkInfoProviderEngineImpl(metricRecorder));
        ThrowIfNull(networkInfoProviderEngineImpl, "networkInfoProviderEngineImplIsNull");
        return networkInfoProviderEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void NetworkInfoProviderEngineImpl::addObserver(std::shared_ptr<NetworkInfoObserver> observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.insert(observer);
}

void NetworkInfoProviderEngineImpl::removeObserver(std::shared_ptr<NetworkInfoObserver> observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.erase(observer);
}

void NetworkInfoProviderEngineImpl::networkInfoChanged(NetworkStatus status, int wifiSignalStrength) {
    AACE_INFO(LX(TAG).d("status", status));
    std::lock_guard<std::mutex> lock(m_mutex);
    bool wasConnected = m_isConnected;
    m_isConnected = status == NetworkStatus::CONNECTED;
    if (wasConnected && !m_isConnected) {
        // Network connection lost. Record metrics to count the disconnection
        // and record the preceding network uptime. Start measuring duration of
        // network downtime.
        std::vector<DataPoint> countDps = {
            CounterDataPointBuilder{}.withName(METRIC_CONNECTION_CHANGE_COUNT).increment(1).build(),
            StringDataPointBuilder{}
                .withName(METRIC_CONNECTION_STATUS_TYPE)
                .withValue(METRIC_STATUS_DISCONNECTED)
                .build()};
        submitMetric(m_metricRecorder, countDps);

        std::vector<DataPoint> durationDps = {
            m_connectedDuration.stopTimer().withName(METRIC_CONNECTION_STATUS_DURATION).build(),
            StringDataPointBuilder{}
                .withName(METRIC_CONNECTION_STATUS_TYPE)
                .withValue(METRIC_STATUS_CONNECTED)
                .build()};
        submitMetric(m_metricRecorder, durationDps);

        m_disconnectedDuration.startTimer();

    } else if (!wasConnected && m_isConnected) {
        // Network connection restored. Record metrics to count the connection
        // and record the preceding network downtime. Start measuring duration
        // of network uptime
        std::vector<DataPoint> countDps = {
            CounterDataPointBuilder{}.withName(METRIC_CONNECTION_CHANGE_COUNT).increment(1).build(),
            StringDataPointBuilder{}
                .withName(METRIC_CONNECTION_STATUS_TYPE)
                .withValue(METRIC_STATUS_CONNECTED)
                .build()};
        submitMetric(m_metricRecorder, countDps);

        std::vector<DataPoint> durationDps = {
            m_disconnectedDuration.stopTimer().withName(METRIC_CONNECTION_STATUS_DURATION).build(),
            StringDataPointBuilder{}
                .withName(METRIC_CONNECTION_STATUS_TYPE)
                .withValue(METRIC_STATUS_DISCONNECTED)
                .build()};
        submitMetric(m_metricRecorder, durationDps);

        m_connectedDuration.startTimer();
    }

    for (const auto& next : m_observers) {
        next->onNetworkInfoChanged(status, wifiSignalStrength);
    }
}

bool NetworkInfoProviderEngineImpl::setNetworkInterface(const std::string& networkInterface) {
    try {
        AACE_INFO(LX(TAG).sensitive("networkInterface", networkInterface));

        std::lock_guard<std::mutex> lock(m_mutex);

        //Notify the begin
        for (const auto& next : m_observers) {
            next->onNetworkInterfaceChangeStatusChanged(
                networkInterface, NetworkInfoObserver::NetworkInterfaceChangeStatus::BEGIN);
        }

        //Notify to Change network interface
        for (const auto& next : m_observers) {
            next->onNetworkInterfaceChangeStatusChanged(
                networkInterface, NetworkInfoObserver::NetworkInterfaceChangeStatus::CHANGE);
        }

        // Notify Completed
        for (const auto& next : m_observers) {
            next->onNetworkInterfaceChangeStatusChanged(
                networkInterface, NetworkInfoObserver::NetworkInterfaceChangeStatus::COMPLETED);
        }

        return true;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool NetworkInfoProviderEngineImpl::setNetworkHttpProxyHeader(const std::string& headers) {
    AACE_INFO(LX(TAG));
    try {
        ThrowIf(headers.empty(), "proxyHeadersEmpty");

        auto headersVector = std::vector<std::string>{};
        auto ss = std::stringstream{headers};

        for (std::string line; std::getline(ss, line, '\n');) {
            headersVector.push_back(line);
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& next : m_observers) {
            next->onNetworkProxyHeadersAvailable(headersVector);
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void NetworkInfoProviderEngineImpl::stop() {
    AACE_INFO(LX(TAG));
    std::unique_lock<std::mutex> lock(m_mutex);
    bool connected = m_isConnected;
    lock.unlock();
    // Stop the downtime or uptime duration timer and record the duration metric
    // since Engine stop generally precedes shutdown. Restart the timer just in
    // case the Engine is started again to continue monitoring duration
    if (connected) {
        std::vector<DataPoint> durationDps = {
            m_connectedDuration.stopTimer().withName(METRIC_CONNECTION_STATUS_DURATION).build(),
            StringDataPointBuilder{}
                .withName(METRIC_CONNECTION_STATUS_TYPE)
                .withValue(METRIC_STATUS_CONNECTED)
                .build()};
        submitMetric(m_metricRecorder, durationDps);
        m_connectedDuration.startTimer();
    } else {
        std::vector<DataPoint> durationDps = {
            m_disconnectedDuration.stopTimer().withName(METRIC_CONNECTION_STATUS_DURATION).build(),
            StringDataPointBuilder{}
                .withName(METRIC_CONNECTION_STATUS_TYPE)
                .withValue(METRIC_STATUS_DISCONNECTED)
                .build()};
        submitMetric(m_metricRecorder, durationDps);
        m_disconnectedDuration.startTimer();
    }
}

}  // namespace network
}  // namespace engine
}  // namespace aace
