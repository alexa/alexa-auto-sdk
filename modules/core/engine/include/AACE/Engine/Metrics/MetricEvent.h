/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <string>
#include <unordered_map>

namespace aace {
namespace engine {
namespace metrics {

class MetricEvent {
public:
    /**
     * An enum class to represent the metric types: One of CT (counter), TI (timer), DV (discrete value)
     */
    enum class MetricDataType { TI, DV, CT };

    /**
     * An enum class to represent upload priority: One of HI (high) or NR (normal). Default is NR.
     */
    enum class MetricPriority { NR, HI };

    /**
     * An enum class to indicate if the metric event should be buffered: One of BF (buffer) or NB (No-buffer). Default is NB.
     */
    enum class MetricBufferType { BF, NB };

    /**
     * An enum class to indicate if the metric event should be tagged with a unique identifier: One of UNIQ (unique) or NUNI (Non-unique). Default is NUNI.
     */
    enum class MetricIdentityType { UNIQ, NUNI };

    /**
     * Constructor.
     *
     * @param program The name that indicates where the event came from / who reported.
     * @param source The name that provides additional contextual information about how the event happened.
     */
    MetricEvent(const std::string& program, const std::string& source);

    /**
     * Constructor.
     *
     * @param program The name that indicates where the event came from / who reported.
     * @param source The name that provides additional contextual information about how the event happened.
     * @param priority The priority of the metric.
     */
    MetricEvent(const std::string& program, const std::string& source, MetricPriority priority);

    /**
     * Constructor.
     *
     * @param program The name that indicates where the event came from / who reported.
     * @param source The name that provides additional contextual information about how the event happened.
     * @param bufferType Enum to indicate if the metric needs to be buffered
     */
    MetricEvent(const std::string& program, const std::string& source, MetricBufferType bufferType);

    /**
     * Constructor.
     *
     * @param program The name that indicates where the event came from / who reported.
     * @param source The name that provides additional contextual information about how the event happened.
     * @param identityType Enum to indicate metric identity type.
     */
    MetricEvent(
        const std::string& program,
        const std::string& source,
        MetricBufferType bufferType,
        MetricIdentityType identityType);

    /**
     * Constructor.
     *
     * @param program The name that indicates where the event came from / who reported.
     * @param source The name that provides additional contextual information about how the event happened.
     */
    MetricEvent(
        const std::string& program,
        const std::string& source,
        MetricPriority priority,
        MetricBufferType bufferType,
        MetricIdentityType identityType);

    /**
     * Add timer data to the metric event.
     *
     * @param name The name describing the datapoint being captured.
     * @param value The time in milliseconds.
     */
    void addTimer(const std::string& name, double value);

    /**
     * Add string data to the metric event. 
     *
     * @param name The name describing the datapoint being captured.
     * @param value The string that represents the value.
     */
    void addString(const std::string& name, const std::string& value);

    /**
     * Add counter data to the metric event. 
     *
     * @param name The name describing the datapoint being captured.
     * @param value The number that represents frequency or count.
     */
    void addCounter(const std::string& name, int value);

    /**
     * Print the metric event data via logger in a standardized metric format.
     */
    void record();

private:
    /**
     * Convert MetricPriority enum to String representation. 
     *
     * @param priority The enum to convert.
     */
    static std::string priorityToString(MetricPriority priority);

    /**
     * Convert MetricDataType enum to String representation. 
     *
     * @param priority The enum to convert.
     */
    static std::string dataTypeToString(MetricDataType dataPoint);

    /**
     * Convert MetricDataType enum to String representation.
     *
     * @param type The buffer type enum to convert.
     */
    static std::string bufferTypeToString(MetricBufferType type);

    /**
     * Convert MetricIdentityType enum to String representation.
     *
     * @param type The metric identity type enum to convert.
     */
    static std::string identityTypeToString(MetricIdentityType type);

    /**
     * Helper method to append data to the string log that is being built. 
     *
     * @param name The name describing the datapoint being captured.
     * @param value The string that represents the value.
     * @param type The type of metric (timer, counter, string).
     * @param sampleCount The frequency or count of the datapoint.
     */
    void addDataToLog(std::string name, std::string value, MetricDataType type, std::string sampleCount);

    /// Name that indicates where the event came from / who reported.
    std::string m_program;

    /// Name that provides additional contextual information about how the event happened.
    std::string m_source;

    /// A string that follows a standard format to capture metric datapoints.
    std::string m_metricLog;

    /// Priority of the metric (High or Normal).
    MetricPriority m_priority;

    /// Buffer type of the metric (Buffer or No-Buffer).
    MetricBufferType m_bufferType;

    /// Identity type of the metric (Unique or Non-unique).
    MetricIdentityType m_identityType;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRIC_EVENT_H
