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

#include <stdexcept>

#include "AACE/Engine/Metrics/AASBMetricsUtils.h"
#include "AACE/Engine/Metrics/MetricsUploadConfiguration.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace metrics {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.MetricsUploadConfiguration");

MetricsUploadConfiguration::MetricsUploadConfiguration(AgentIdType agentId) : m_agentId{agentId} {
}

MetricsUploadConfiguration::MetricsUploadConfiguration(
    AgentIdType agentId,
    const std::unordered_map<std::string, MetricUploadConfig>& allowedMetrics) :
        m_agentId{agentId}, m_allowedMetrics{allowedMetrics} {
}

bool MetricsUploadConfiguration::isAllowed(const MetricEvent& metricEvent) const {
    std::string key = metricEvent.getProgramName() + "%" + metricEvent.getSourceName();
    auto metricIter = m_allowedMetrics.find(key);
    if (metricIter == m_allowedMetrics.end()) {
        AACE_DEBUG(LX(TAG).m("Key not found in allow list").d("agentId", m_agentId).d("key", key));
        return false;
    }
    const MetricUploadConfig& uploadConfig = metricIter->second;
    if (uploadConfig.nameRgxList.empty()) {
        return true;
    }
    for (const auto& dp : metricEvent.getDataPoints()) {
        const std::string dpName = dp.getName();
        for (const std::regex& nameRgx : uploadConfig.nameRgxList) {
            bool found = std::regex_search(dpName, nameRgx);
            if (found) {
                AACE_VERBOSE(LX(TAG).m("Matched data point").d("key", key).d("name", dpName));
                return true;
            }
        }
    }
    AACE_DEBUG(LX(TAG).m("No matching data point filter for metric").d("agentId", m_agentId).d("key", key));
    return false;
}

void MetricsUploadConfiguration::addMetricConfig(const std::string key, const MetricUploadConfig& config) {
    m_allowedMetrics.insert({key, config});
}

std::unordered_map<AgentIdType, MetricsUploadConfiguration> MetricsUploadConfiguration::createConfigurations(
    const json& uploadConfig) {
    AACE_DEBUG(LX(TAG).d("numberOfMetricConfigEntries", uploadConfig.size()));
    std::unordered_map<AgentIdType, MetricsUploadConfiguration> configs;
    for (const auto& itr : uploadConfig.items()) {
        const json& item = itr.value();
        const std::string program = item.at("program");
        const std::string source = item.at("source");
        const json& uploadRules = item.at("uploadRules");
        AACE_VERBOSE(LX(TAG)
                         .m("Creating config")
                         .d("program", program)
                         .d("source", source)
                         .d("numberOfRules", uploadRules.size()));
        for (const auto& ruleItr : uploadRules.items()) {
            const json& rule = ruleItr.value();
            AgentIdType agent = rule.at("assistantId");
            std::string group;
            std::string schema;
            std::vector<std::regex> names;
            if (rule.contains("groupId")) {
                group = rule.at("groupId");
                schema = rule.at("schemaId");
            }
            if (rule.contains("names")) {
                auto namesJ = rule.at("names");
                if (!namesJ.is_array()) {
                    throw std::invalid_argument(
                        "uploaderConfig.metricConfigs[i].uploadRules[j].names must be an array");
                }
                names = namesJ.get<std::vector<std::regex>>();
                AACE_VERBOSE(LX(TAG)
                                 .m("Metric has names filter")
                                 .d("program", program)
                                 .d("source", source)
                                 .d("numNameFilters", names.size()));
            }
            const MetricUploadConfig config(program, source, names, group, schema);
            const std::string key = config.program + "%" + config.source;
            const auto agentConfigItr = configs.find(agent);
            if (agentConfigItr != configs.end()) {
                agentConfigItr->second.addMetricConfig(key, config);
            } else {
                std::unordered_map<std::string, MetricUploadConfig> newEntry;
                newEntry.insert({key, config});
                const MetricsUploadConfiguration newConfig(agent, newEntry);
                configs.insert({agent, newConfig});
            }
        }
    }
    return configs;
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
