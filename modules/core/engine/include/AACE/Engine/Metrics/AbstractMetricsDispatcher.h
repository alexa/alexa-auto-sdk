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

#ifndef AACE_ENGINE_METRICS_ABSTRACT_METRICS_DISPATCHER_H
#define AACE_ENGINE_METRICS_ABSTRACT_METRICS_DISPATCHER_H

#include <mutex>
#include <queue>
#include <vector>

#include <AACE/Engine/Metrics/MetricsDispatcherInterface.h>

namespace aace {
namespace engine {
namespace metrics {

static constexpr unsigned int DEFAULT_PRE_DISPATCH_BUFFER_SIZE = 200;

/**
 * The @c AbstractMetricsDispatcher implements the @c MetricsDispatcherInterface
 * including a pre-dispatch buffering for agents that have conditions to meet to
 * permit uploading metrics. The implementation buffers metrics in a FIFO
 * buffer prior to sending the metrics to the uploader when required conditions
 * are met.
 */
class AbstractMetricsDispatcher : public MetricsDispatcherInterface {
public:
    /**
     * Destructor
     */
    ~AbstractMetricsDispatcher() = default;

    /// aace::engine::metrics::MetricsDispatcherInterface
    /// @{
    bool hasPreDispatchRules() override;
    void submitMetric(const MetricEvent& metricEvent) override;
    void prepareForShutdown() override;
    void shutdown() override;
    void onMetricEmissionStateChanged(bool emit) override;
    /// @}

protected:
    /**
     * Constructor
     *
     * @param agentId The agent ID of the agent associated with the metrics
     * @param hasPreDispatchRules Whether this @c AbstractMetricsDispatcher 
     *        has additional conditions to meet before dispatching metrics.
     *        If @c true, @c onMetricEmissionStateChanged controls the
     *        enablement.
     * @param maxMetrics The maximum number of metrics to buffer before
     *        dispatch conditions are met. Must be a positive integer. Older
     *        metrics will be dropped if @a hasPreDispatchRules is true and the
     *        buffer reaches capacity before @c onMetricEmissionStateChanged
     *        has enabled dispatch.
     */
    AbstractMetricsDispatcher(unsigned int agentId, bool hasPreDispatchRules, unsigned int maxMetrics);

private:
    /**
     * Dispatch the @c MetricEvent to the uploader.
     *
     * @note The implementation must be non-blocking.
     * @param metricEvent The metric event to dispatch
     */
    virtual void dispatchMetric(const MetricEvent& metricEvent) = 0;

    /**
     * Dispatch multiple @c MetricEvent to the uploader.
     *
     * @note The implementation must be non-blocking.
     * @param metricEvents The metric events to dispatch
     */
    virtual void dispatchMetrics(const std::vector<MetricEvent>& metricEvents) = 0;

    /**
     * Immediately dispatch all metrics in the dispatch queue.
     *
     * @note Any metrics in a buffer waiting for dispatching to be enabled must
     * not be published.
     * @note The implementation must be non-blocking.
     */
    virtual void flush() = 0;

    /**
     * Clean up to prepare for deletion, such as for Engine shutdown.
     */
    virtual void cleanup() = 0;

    /**
     * Helper function to add a metric to the pre-dispatch buffer.
     */
    void bufferMetric(const MetricEvent& metricEvent);

    /**
     * A FIFO buffer for metrics recorded prior to required dispatch conditions.
     * Access serialized by @c m_mutex.
     */
    std::queue<MetricEvent> m_buffer;

protected:
    /// The agent ID of the agent associated with the metrics
    unsigned int m_agentId;

    /**
     * Whether the agent for this dispatcher has preconditions to meet before
     * it is allowed to dispatch metrics. If yes,
     * @c onMetricEmissionStateChanged controls the dispatch enablement. 
     */
    bool m_hasPreDispatchRules;

    /**
     * Maximum number of metrics in the pre-dispatch buffer used before dispatch
     * is enabled
     */
    unsigned int m_maxMetricsPreDispatch;

    /**
     * Whether the @c AbstractMetricsDispatcher is allowed to dispatch metrics.
     * @c false if metrics must be buffered until dispatch is allowed. Access
     * serialized by by @c m_dispatchEnabledMutex.
     */
    bool m_dispatchEnabled;

    /**
     * Mutex to serialize access to @c m_dispatchEnabled and @c MetricEvent
     * buffers
     */
    std::mutex m_mutex;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_ABSTRACT_METRICS_DISPATCHER_H
