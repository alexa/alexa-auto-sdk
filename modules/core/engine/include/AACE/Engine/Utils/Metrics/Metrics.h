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

#ifndef AACE_ENGINE_UTILS_METRICS_METRIC_H_
#define AACE_ENGINE_UTILS_METRICS_METRIC_H_

#include <string>
#include <sstream>
#include <memory>
#include <vector>

#include <AACE/Engine/Metrics/MetricEvent.h>

namespace aace {
namespace engine {
namespace utils {
namespace metrics {

using namespace aace::engine::metrics;
/**
 * Emit a counter metric for each datapoint specified in the @ datapoints.
 * Counter is 1 for each metric.
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param datapoints List of datapoint strings to be added to the metric.
 * @param bufferType Enum to indicate whether to buffer this metric. Default is NO-BUFFER.
 * @param identityType Enum to indicate whether to add unique identifier to this metric. Default is NON-UNIQUE.
 */
void emitCounterMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::vector<std::string>& datapoints,
    MetricEvent::MetricBufferType bufferType = MetricEvent::MetricBufferType::NB,
    MetricEvent::MetricIdentityType identityType = MetricEvent::MetricIdentityType::NUNI);

/**
 * Emit a unique counter metric for each datapoint specified in the @ datapoints.
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param datapoints List of datapoint strings to be added to the metric.
 */
void emitUniqueCounterMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::vector<std::string>& datapoints);

/**
 * Emit a counter metric for each datapoint specified in the @ datapoints that will be buffered
 * if user is not authorized
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param datapoints List of datapoint strings to be added to the metric.
 */
void emitBufferedCounterMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::vector<std::string>& datapoints);

/**
 * Emit a counter metric with key as @c key and value as @c value
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param key Metric Name
 * @param value Counter value of the metric
 * @param bufferType Enum to indicate whether to buffer this metric. Default is NO-BUFFER.
 * @param identityType Enum to indicate whether to add unique identifier to this metric. Default is NON-UNIQUE.
 */
void emitCounterMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::string& key,
    const int value,
    MetricEvent::MetricBufferType bufferType = MetricEvent::MetricBufferType::NB,
    MetricEvent::MetricIdentityType identityType = MetricEvent::MetricIdentityType::NUNI);

/**
 * Emit a unique counter metric with key as @c key and value as @c value
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param key Metric Name
 * @param value Counter value of the metric
 */
void emitUniqueCounterMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::string& key,
    const int value);

/**
 * Emit a counter metric with key as @c key and value as @c value that will be buffered
 * if user is not authorized
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param key Metric Name
 * @param value Counter value of the metric
 */
void emitBufferedCounterMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::string& key,
    const int value);

/**
 * Emit a timer metric with key as @c key and value as @c value
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param key Metric Name
 * @param value Timer value of the metric
 * @param bufferType Enum to indicate whether to buffer this metric. Default is NO-BUFFER.
 * @param identityType Enum to indicate whether to add unique identifier to this metric. Default is NON-UNIQUE.
 */
void emitTimerMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::string& key,
    const double value,
    MetricEvent::MetricBufferType bufferType = MetricEvent::MetricBufferType::NB,
    MetricEvent::MetricIdentityType identityType = MetricEvent::MetricIdentityType::NUNI);

/**
 * Emit a unique timer metric with key as @c key and value as @c value
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param key Metric Name
 * @param value Timer value of the metric
 */
void emitUniqueTimerMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::string& key,
    const double value);

/**
 * Emit a timer metric with key as @c key and value as @c value that will be buffered
 * and emitted once the device is authorized.
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param key Metric Name
 * @param value Timer value of the metric
 */
void emitBufferedTimerMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::string& key,
    const double value);

/**
 * Emit metrics for each datapoint specified in the @ counterDatapoints, @c stringDatapoints and @c timerDatapoints.
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param counterDatapoints Datapoints of type COUNTER.
 * @param stringDatapoints Timer Datapoints of type STRING.
 * @param timerDatapoints Timer Datapoints of type TIMER.
 * @param bufferType Enum to indicate whether to buffer this metric. Default is NO-BUFFER.
 * @param identityType Enum to indicate whether to add unique identifier to this metric. Default is NON-UNIQUE.
 */
void emitMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::vector<std::pair<std::string, int>>& counterDatapoints,
    const std::vector<std::pair<std::string, std::string>>& stringDatapoints,
    const std::vector<std::pair<std::string, double>>& timerDatapoints,
    MetricEvent::MetricBufferType bufferType = MetricEvent::MetricBufferType::NB,
    MetricEvent::MetricIdentityType identityType = MetricEvent::MetricIdentityType::NUNI);

/**
 * Emit unique metrics for each datapoint specified in the @ counterDatapoints, @c stringDatapoints and @c timerDatapoints.
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param counterDatapoints Datapoints of type COUNTER. Default is empty array
 * @param stringDatapoints Timer Datapoints of type STRING. Default is empty array
 * @param timerDatapoints Timer Datapoints of type TIMER. Default is empty array
 */
void emitUniqueMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::vector<std::pair<std::string, int>>& counterDatapoints = {},
    const std::vector<std::pair<std::string, std::string>>& stringDatapoints = {},
    const std::vector<std::pair<std::string, double>>& timerDatapoints = {});

/**
 * Emit metrics for each datapoint specified in the @ counterDatapoints, @c stringDatapoints and @c timerDatapoints that will be buffered and emitted once device is authorized with a unique identifier.
 *
 * @param metricSuffix The suffix to add to the metric program name.
 * @param methodName The name of the method that emits this metric.
 * @param counterDatapoints Datapoints of type COUNTER. Default is empty array
 * @param stringDatapoints Timer Datapoints of type STRING. Default is empty array
 * @param timerDatapoints Timer Datapoints of type TIMER. Default is empty array
 */
void emitBufferedMetrics(
    const std::string& metricSuffix,
    const std::string& methodName,
    const std::vector<std::pair<std::string, int>>& counterDatapoints = {},
    const std::vector<std::pair<std::string, std::string>>& stringDatapoints = {},
    const std::vector<std::pair<std::string, double>>& timerDatapoints = {});

/**
 * Get the current time in  milliseconds
 */
double getCurrentTimeInMs();

}  // namespace metrics
}  // namespace utils
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_UTILS_METRICS_METRIC_H_
