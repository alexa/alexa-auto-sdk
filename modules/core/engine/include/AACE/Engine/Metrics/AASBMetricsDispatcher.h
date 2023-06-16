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

#ifndef AACE_ENGINE_METRICS_AASB_METRICS_DISPATCHER_H
#define AACE_ENGINE_METRICS_AASB_METRICS_DISPATCHER_H

#include <AACE/Engine/MessageBroker/MessageBrokerInterface.h>
#include <AACE/Engine/Metrics/AbstractMetricsDispatcher.h>
#include <AACE/Engine/Utils/Timing/Timer.h>

namespace aace {
namespace engine {
namespace metrics {

static constexpr unsigned int DEFAULT_AASB_METRICS_PUBLISH_SECONDS = 60;
static constexpr unsigned int DEFAULT_AASB_MIN_METRICS_FOR_PUBLISH = 25;

/**
 * @c AASBMetricsDispatcher implements the abstract functions of 
 * @c MetricsDispatcher by publishing metrics to the uploader as AASB messages.
 */
class AASBMetricsDispatcher : public AbstractMetricsDispatcher {
public:
    /**
     * Destructor
     */
    ~AASBMetricsDispatcher() = default;

    /**
     * Create an @c AASBMetricsDispatcher.
     *
     * @param messageBroker The MessageBroker that publishes AASB messages.
     * @param agentId The agent ID of the agent associated with the metrics
     * @param hasPreDispatchRules Whether this @c AbstractMetricsDispatcher 
     *        has additional conditions to meet before dispatching metrics.
     *        If @c true, @c onMetricEmissionStateChanged controls the
     *        enablement.
     * @param maxMetricsInBuffer The maximum number of metrics to buffer before
     *        dispatch conditions are met. Must be a positive integer. Older
     *        metrics will be dropped if @a hasPreDispatchRules is true and the
     *        buffer reaches capacity before @c onMetricEmissionStateChanged
     *        has enabled dispatch.
     * @param publishPeriod The number of seconds to wait between publishing
     *        AASB messages if the dispatch buffer isn't at capacity. Must be a
     *        positive integer.
     * @param minMetricsInMessage The the number of metrics to accumulate in
     *        the dispatch buffer prior to publishing in an AASB message. Must
     *        be a positive integer. The buffer will still publish at partial
     *        capacity if @a publishPeriod elapses.
     * @return A unique_ptr to an @c AASBMetricsDispatcher or nullptr if creation fails
     */
    static std::unique_ptr<AASBMetricsDispatcher> create(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
        unsigned int agentId,
        bool hasPreDispatchRules,
        unsigned int maxMetricsInBuffer = DEFAULT_PRE_DISPATCH_BUFFER_SIZE,
        unsigned int publishPeriod = DEFAULT_AASB_METRICS_PUBLISH_SECONDS,
        unsigned int minMetricsInMessage = DEFAULT_AASB_MIN_METRICS_FOR_PUBLISH);

private:
    /// aace::engine::metrics::AbstractMetricsDispatcher
    /// @{
    void dispatchMetric(const MetricEvent& metricEvent) override;
    void dispatchMetrics(const std::vector<MetricEvent>& metricEvents) override;
    void flush() override;
    void cleanup() override;
    /// @}

    /**
     * Constructor
     *
     * @param messageBroker The MessageBroker that publishes AASB messages.
     * @param agentId The agent ID of the agent associated with the metrics
     * @param hasPreDispatchRules Whether this @c AbstractMetricsDispatcher 
     *        has additional conditions to meet before dispatching metrics.
     *        If @c true, @c onMetricEmissionStateChanged controls the
     *        enablement.
     * @param maxMetricsInBuffer The maximum number of metrics to buffer before
     *        dispatch conditions are met. Must be a positive integer. Older
     *        metrics will be dropped if @a hasPreDispatchRules is true and the
     *        buffer reaches capacity before @c onMetricEmissionStateChanged
     *        has enabled dispatch.
     * @param publishPeriod The number of seconds to wait between publishing
     *        AASB messages if the dispatch buffer isn't at capacity. Must be a
     *        positive integer.
     * @param minMetricsInMessage The the number of metrics to accumulate in
     *        the dispatch buffer prior to publishing in an AASB message. Must
     *        be a positive integer. The buffer will still publish at partial
     *        capacity if @a publishPeriod elapses.
     */
    AASBMetricsDispatcher(
        std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker,
        unsigned int agentId,
        bool hasPreDispatchRules,
        unsigned int maxMetricsInBuffer,
        unsigned int publishPeriod,
        unsigned int minMetricsInMessage);

    /**
     * Create the AASB message containing the metrics in @c m_dispatchBuffer 
     * and publish it with @c MessageBroker. Calling thead must already hold
     * @c m_dispatchMutex.
     */
    void publishBufferAsAASBLocked();

    /**
     * Depending on the size of @c m_dispatchBuffer, publish the buffer
     * contents or start the publish timer. Calling thead must already hold
     * @c m_dispatchMutex.
     */
    void publishWhenReadyLocked();

    /**
     * Start @c m_dispatchTimer to publish metrics after @c m_publishSeconds.
     */
    void startTimer();

    /**
     * Publish the @c m_dispatchBuffer contents when @c m_dispatchTimer period
     * elapses.
     */
    void publishPeriodElapsed();

    /// The MessageBroker
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> m_messageBroker;

    /// The publish period in seconds
    unsigned int m_publishSeconds;

    /**
     * The minimum number of metrics per AASB message unless publish is
     * triggered explicitly, such as timer elapsing or flushing the buffer.
     */
    unsigned int m_minMetricsInMessage;

    /** 
     * A buffer of metrics ready for dispatch. Access serialized by
     * @c m_dispatchMutex.
     */
    std::vector<MetricEvent> m_dispatchBuffer;

    /// Serializes access to @c m_dispatchMutex.
    std::mutex m_dispatchMutex;

    /// Timer to track when to publish the metrics AASB message
    aace::engine::utils::timing::Timer m_dispatchTimer;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_AASB_METRICS_DISPATCHER_H
