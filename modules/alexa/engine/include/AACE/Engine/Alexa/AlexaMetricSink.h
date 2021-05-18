/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_METRIC_SINK_H
#define AACE_ENGINE_METRIC_SINK_H

#include <string>

#include <AVSCommon/Utils/Metrics/MetricSinkInterface.h>
#include <AVSCommon/Utils/Metrics/MetricEvent.h>

namespace aace {
namespace engine {
namespace alexa {

/**
 * This class implements @c MetricSinkInterface to capture AVS Device SDK metric events
 */
class AlexaMetricSink
        : public alexaClientSDK::avsCommon::utils::metrics::MetricSinkInterface
        , public std::enable_shared_from_this<AlexaMetricSink> {
public:
    /**
     * Destructor.
     */
    virtual ~AlexaMetricSink() = default;

    /// @name MetricSinkInterface
    /// @{
    void consumeMetric(std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricEvent> metricEvent) override;
    /// @}

private:
    void emitAllDatapointsMetric(std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricEvent> metricEvent);
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRIC_SINK_H
