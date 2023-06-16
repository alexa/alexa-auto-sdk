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

#ifndef AACE_ENGINE_METRICS_METRIC_EVENT_BUILDER_H
#define AACE_ENGINE_METRICS_METRIC_EVENT_BUILDER_H

#include <chrono>
#include <exception>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <AACE/Engine/Metrics/MetricEvent.h>

namespace aace {
namespace engine {
namespace metrics {

class MetricEventBuilder {
public:
    /**
     * Constructor.
     */
    MetricEventBuilder();

    /**
     * Sets the program name for the metric event. A product name, application,
     * or service to which the metric belongs.
     * If this function is not called, the builder will use the default
     * program name "AlexaAutoSDK".
     *
     * @param programName The program name for the metric event
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withProgramName(const std::string& programName);

    /**
     * Sets the source name for the metric event. A method or other software
     * component identifying the source of the metric. The combination of
     * program and source uniquely identify the metric.
     *
     * @param sourceName The source name for the metric event
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withSourceName(const std::string& sourceName);

    /**
     * Sets the priority for uploading the metric event.
     * If this function is not called, the builder will use the default normal
     * priority.
     *
     * @param priority The priority for the metric event
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withPriority(Priority priority);

    /**
     * Sets the buffer behavior for the metric before terms and conditions
     * acceptance.
     * If this function is not called, the builder will use the default
     * buffer type, which allows the uploader implementation to drop the metric
     * if it is recorded before the user accepts terms and conditions.
     *
     * @param bufferType The buffer type for the metric event
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withBufferType(BufferType bufferType);

    /**
     * Sets the identity type for the metric. 
     * If this function is not called, the builder will use the default
     * identity type.
     *
     * @param identityType The identity type for the metric event
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withIdentityType(IdentityType identityType);

    /**
     * Sets the ID of the agent associated with the metric.
     * If this function is not called, the builder will use AGENT_ID_NONE so
     * the metric is not associated with any agent.
     *
     * @param agentId The agent ID associated with the metric event
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withAgentId(AgentIdType agentId);

    /**
     * Associates the metric with Alexa's agent ID.
     *
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withAlexaAgentId();

    /**
     * Associates the metric with all agents.
     *
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withAllAgentId();

    /**
     * Associates the metric with no agents.
     *
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withNoneAgentId();

    /**
     * Sets the timestamp of the metric. Use this function if the metric was
     * created at a time other than when the builder is used. If this function
     * is not called, the builder will use the time when @c build() is called.
     *
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& withTimeStamp(std::chrono::steady_clock::time_point timestamp);

    /**
     * Adds a key/value pair associated with the metric event. The meaning of
     * the key is contractual between metric owner and the uploader
     * implementation.
     * If the specified key is already added to the metric, the existing one
     * is preferred.
     *
     * @param key The key of the metadata entry
     * @param value The value of the metadata entry
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& addMetadata(const std::string& key, const std::string& value);

    /**
     * Adds key/value pairs associated with the metric event. The meaning of
     * the keys are contractual between metric owner and the uploader
     * implementation.
     * If any specified key is already added to the metric, the existing one
     * is preferred.
     *
     * @param metadata The map of key value pairs to add
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& addMetadata(std::unordered_map<std::string, std::string> metadata);

    /**
     * Adds a data point to the metric event.
     * Each DataPoint must have a unique name. Repeated names will overwrite
     * the previous DataPoint of the same name.
     *
     * @param dataPoint The DataPoint object to add to the metric
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& addDataPoint(const DataPoint& dataPoint);

    /**
     * Adds multiple data points to the metric event.
     * Each DataPoint must have a unique name. Repeated names will overwrite
     * the previous DataPoint of the same name.
     *
     * @param dataPoint The DataPoint objects to add to the metric
     * @return The metric builder instance for chaining calls
     */
    MetricEventBuilder& addDataPoints(const std::vector<DataPoint>& dataPoints);

    /**
     * Builds the @c MetricEvent.
     *
     * @return A @c MetricEvent object
     * @throw @c std::invalid_argument exception if the MetricEvent cannot be
     *        built due to improper use of the builder
     */
    MetricEvent build();

private:
    /// The program name of the metric.
    std::string m_programName;

    /// The source name of the metric.
    std::string m_sourceName;

    /// The agent ID associated with the metric.
    AgentIdType m_agentId;

    /// The priority of the metric
    Priority m_priority;

    /// The buffer type of the metric
    BufferType m_bufferType;

    /// The identity type of the metric
    IdentityType m_identityType;

    /// Whether a custom timestamp was specified
    bool m_overloadTimestamp;

    /// The timestamp of the metric
    std::chrono::steady_clock::time_point m_timestamp;

    /// The optional metadata of the metric
    std::unordered_map<std::string, std::string> m_metadata;

    /// The map of data points of the metric event, keyed by data point name
    std::unordered_map<std::string, DataPoint> m_dataPoints;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRIC_EVENT_BUILDER_H
