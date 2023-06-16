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

#ifndef AACE_ENGINE_METRICS_METRICS_UPLOAD_CONFIGURATION_H
#define AACE_ENGINE_METRICS_METRICS_UPLOAD_CONFIGURATION_H

#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "AACE/Engine/Metrics/MetricEvent.h"
#include "AACE/Engine/Utils/Agent/AgentId.h"

using AgentIdType = aace::engine::utils::agent::AgentIdType;
using json = nlohmann::json;

namespace aace {
namespace engine {
namespace metrics {

/**
 * MetricUploadConfig holds upload configuration for an individual
 * program+source pair. 
*/
struct MetricUploadConfig {
    /// The metric program
    std::string program;
    /// The metric source name
    std::string source;
    /**
     * A list of data point name regex filters. The metric is allowed if any
     * of its data point names match any filter in the list. Empty list
     * means the metric has no restrictions on required data points.
     */
    std::vector<std::regex> nameRgxList;
    /// The upload group ID
    std::string groupId;
    /// The upload schema ID
    std::string schemaId;

    MetricUploadConfig(
        const std::string& program,
        const std::string& source,
        const std::vector<std::regex>& nameRgxList = {},
        const std::string& groupId = "",
        const std::string& schemaId = "") :
            program{program}, source{source}, nameRgxList{nameRgxList}, groupId{groupId}, schemaId{schemaId} {
    }
};

/**
 * A @c MetricsUploadConfiguration holds rules regarding which metrics may be
 * recorded for a particular agent.
 */
class MetricsUploadConfiguration {
public:
    /// Destructor
    ~MetricsUploadConfiguration() = default;

    /**
     * Check whether this @c MetricsUploadConfiguration allows the specified
     * @c MetricEvent to be uploaded for the agent corresponding to the 
     * @c MetricsUploadConfiguration.
     * 
     * @param metricEvent The metric to check
     * @return @c true if the metric may be uploaded or false if it must be
     *         dropped
    */
    bool isAllowed(const MetricEvent& metricEvent) const;

    /**
     * Generates a @c MetricsUploadConfiguration for each agent present in the
     * specified configuration
     * 
     * @param uploadConfig The "metricConfigs" JSON array of aace.metrics
     *        Engine configuration
     * @return The configurations as a map, keyed by agent ID
     * @throws std::exception if the specified config could not be parsed
     */
    static std::unordered_map<AgentIdType, MetricsUploadConfiguration> createConfigurations(const json& uploadConfig);

    /**
     * Get the number of allowed metrics in this @c MetricsUploadConfiguration.
     * @return The number of metric configurations
     */
    inline size_t size() const {
        return m_allowedMetrics.size();
    }

private:
    /**
     * Constructor
     */
    MetricsUploadConfiguration(AgentIdType agentId);

    /**
     * Constructor
     */
    MetricsUploadConfiguration(
        AgentIdType agentId,
        const std::unordered_map<std::string, MetricUploadConfig>& allowedMetrics);

    /**
     * Add a single metric configuration to the @c MetricsUploadConfiguration
     * 
     * @param key The key for storing the map. Must be of the form
     *        "<program>%<source>".
     * @param config The metric config to add
    */
    void addMetricConfig(const std::string key, const MetricUploadConfig& config);

    /// The agent ID of the agent associated with this configuration.
    AgentIdType m_agentId;

    /*
     * The map of metric configurations allowed for the agent. Keys use the
     * format "<program>%<source>".
    */
    std::unordered_map<std::string, MetricUploadConfig> m_allowedMetrics;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_METRICS_UPLOAD_CONFIGURATION_H