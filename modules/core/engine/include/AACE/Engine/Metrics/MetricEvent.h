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

#ifndef AACE_ENGINE_METRICS_METRIC_EVENT_H
#define AACE_ENGINE_METRICS_METRIC_EVENT_H

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

#include <AACE/Engine/Metrics/DataPoint.h>
#include <AACE/Engine/Metrics/MetricContext.h>

namespace aace {
namespace engine {
namespace metrics {

class MetricEvent {
public:
    /**
     * Constructor.
     *
     * @param programName The program name of the metric. A product name,
     *        application, or service to which the metric belongs.
     * @param sourceName The source name of the metric. A method or other 
     *        software component identifying the source of the metric. The 
     *        combination of program and source uniquely identify the metric.
     * @param metricContext The contextual properties related to recording the
     *        metric.
     * @param dataPoints A map of @c DataPoint objects, keyed by data point
     *        name. Data points in the map are numeric values and optional
     *        string dimensions comprising the metric.
     * @param timestamp The timestamp at which this metric event was created.
     */
    MetricEvent(
        const std::string& programName,
        const std::string& sourceName,
        MetricContext metricContext,
        const std::unordered_map<std::string, DataPoint>& dataPoints,
        std::chrono::steady_clock::time_point timestamp);

    /**
     * Get the program name of the metric.
     *
     * @return The program name
     */
    std::string getProgramName() const;

    /**
     * Get the source name of the metric.
     *
     * @return The source name
     */
    std::string getSourceName() const;

    /**
     * Get the @c MetricContext of the metric event.
     *
     * @return The @c MetricContext
     */
    const MetricContext& getMetricContext() const;

    /**
     * Get a @c DataPoint object by name
     *
     * @param name The name of the data point
     * @param dataType The @c DataType of the data point
     * @return The data point. @c Datapoint::isValid() returns false if the
     *         requested name does not exist in the set of data points.
     */
    DataPoint getDataPoint(const std::string& name, DataType dataType) const;

    /**
     * Get the list of data points for the metric event.
     *
     * @return The list of @c DataPoint
     */
    std::vector<DataPoint> getDataPoints() const;

    /**
     * Get the timestamp of when the metric event was created as a system clock
     * time point.
     *
     * @return The timestamp of creation
     */
    std::chrono::system_clock::time_point getSystemClockTimestamp() const;

    /**
     * Get the timestamp of when the metric event was created as a steady clock
     * time point.
     *
     * @return The timestamp of creation
     */
    std::chrono::steady_clock::time_point getSteadyClockTimestamp() const;

private:
    /**
     * The program name of the metric. A product name, application, or service
     * to which the metric belongs.
     */
    std::string m_programName;

    /**
     * The source name of the metric. A method or other software component
     * identifying the source of the metric. The combination of program and
     * source uniquely identify the metric.
     */
    std::string m_sourceName;

    /// The metric context of the metric event
    MetricContext m_metricContext;

    /// The map of data points of the metric event, keyed by data point name
    std::unordered_map<std::string, DataPoint> m_dataPoints;

    /// The timestamp of when the metric event was created
    std::chrono::steady_clock::time_point m_timestamp;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRIC_EVENT_H
