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

#ifndef AACE_ENGINE_METRICS_METRICS_DISPATCHER_INTERFACE_H
#define AACE_ENGINE_METRICS_METRICS_DISPATCHER_INTERFACE_H

#include <AACE/Engine/Metrics/MetricEvent.h>
#include <AACE/Engine/Metrics/MetricsEmissionListenerInterface.h>

namespace aace {
namespace engine {
namespace metrics {

/**
 * The @c MetricsDispatcherInterface implementation is responsible for
 * dispatching metrics to the uploader.
 */
class MetricsDispatcherInterface : public aace::engine::metrics::MetricsEmissionListenerInterface {
public:
    /**
     * Destructor
     */
    virtual ~MetricsDispatcherInterface() = default;

    /**
     * Checks whether the metrics dispatcher for the agent has runtime
     * preconditions that must be met before the dispatcher is allowed to
     * dispatch the agent's metrics. If yes, use @c onMetricEmissionStateChanged
     * to control the enablement.
     */
    virtual bool hasPreDispatchRules() = 0;

    /**
     * Prepare the metric for dispatch to the uploader. Buffer the metric or
     * dispatch if all required conditions are met.
     *
     * @param metricEvent The metric event
     */
    virtual void submitMetric(const MetricEvent& metricEvent) = 0;

    /**
     * Perform any necessary cleanup prior to a potential Engine shutdown,
     * such as flushing a buffer to the uploader.
     */
    virtual void prepareForShutdown() = 0;

    /*
     * Perform any necessary cleanup at Engine shutdown.
     */
    virtual void shutdown() = 0;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRICS_DISPATCHER_INTERFACE_H
