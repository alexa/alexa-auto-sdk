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

#ifndef AACE_ENGINE_METRICS_METRIC_RECORDER_INTERFACE_H
#define AACE_ENGINE_METRICS_METRIC_RECORDER_INTERFACE_H

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/MetricEvent.h>

namespace aace {
namespace engine {
namespace metrics {

/**
 * An interface to record metrics. The @c MetricRecorderServiceInterface is responsible
 * for applying any preprocessing before upload and routing the submitted
 * metric events to an uploader.
 */
class MetricRecorderServiceInterface {
public:
    /**
     * Destructor
     */
    virtual ~MetricRecorderServiceInterface() = default;

    /**
     * Record a @c MetricEvent so it routes to an uploader.
     *
     * @note The implementation must be non-blocking.
     * @param metricEvent The metric event to record
     */
    virtual void recordMetric(const MetricEvent& metricEvent) = 0;
};

/**
 * Record the specified @c MetricEvent with the
 * specified @c MetricRecorderServiceInterface.
 *
 * @param recorder The MetricRecorderServiceInterface to use
 * @param metricEvent The MetricEvent to record
 */
inline void recordMetric(const std::shared_ptr<MetricRecorderServiceInterface>& recorder, MetricEvent metricEvent) {
    if (recorder == nullptr) {
        AACE_WARN(LX("MetricRecorderServiceInterface")
                      .m("Cannot record metric with null recorder; dropping")
                      .d("program", metricEvent.getProgramName())
                      .d("source", metricEvent.getSourceName()));
        return;
    }
    recorder->recordMetric(std::move(metricEvent));
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRIC_RECORDER_INTERFACE_H
