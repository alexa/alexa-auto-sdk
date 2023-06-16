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

#ifndef AACE_ENGINE_ALEXA_AVS_SDK_METRIC_SINK_H
#define AACE_ENGINE_ALEXA_AVS_SDK_METRIC_SINK_H

#include <memory>
#include <string>

#include <AACE/Engine/Alexa/AvsSdkMetricParser.h>
#include <AACE/Engine/Metrics/MetricRecorderServiceInterface.h>
#include <AVSCommon/Utils/Metrics/MetricSinkInterface.h>
#include <AVSCommon/Utils/Metrics/MetricEvent.h>

namespace aace {
namespace engine {
namespace alexa {

/**
 * This class implements @c MetricSinkInterface to capture metrics emitted by
 * AVS Device SDK.
 */
class AvsSdkMetricSink
        : public alexaClientSDK::avsCommon::utils::metrics::MetricSinkInterface
        , public std::enable_shared_from_this<AvsSdkMetricSink> {
public:
    /**
     * Create an @c AvsSdkMetricSink.
     * @return A unique pointer to an @c AvsSdkMetricSink or nullptr if creation
     *         failed.
     */
    static std::unique_ptr<AvsSdkMetricSink> create(
        std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder);

    /// @name MetricSinkInterface
    /// @{
    void consumeMetric(std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricEvent> metricEvent) override;
    /// @}

private:
    /**
     * Constructor.
     */
    AvsSdkMetricSink(std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> metricRecorder);

    /// Initialize the AvsSdkMetricSink.
    bool initialize();

    /// Record AVS Metric log
    void logMetric(aace::engine::metrics::MetricEvent& metricEvent);

    std::shared_ptr<aace::engine::metrics::MetricRecorderServiceInterface> m_metricRecorder;
    AvsSdkMetricParser m_parser;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_AVS_SDK_METRIC_SINK_H
