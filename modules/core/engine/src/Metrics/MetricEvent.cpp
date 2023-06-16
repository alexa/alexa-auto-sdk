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

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Metrics/MetricEvent.h"

namespace aace {
namespace engine {
namespace metrics {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.MetricEvent");

MetricEvent::MetricEvent(
    const std::string& programName,
    const std::string& sourceName,
    MetricContext metricContext,
    const std::unordered_map<std::string, DataPoint>& dataPoints,
    std::chrono::steady_clock::time_point timestamp) :
        m_programName{programName},
        m_sourceName{sourceName},
        m_metricContext{std::move(metricContext)},
        m_dataPoints{dataPoints},
        m_timestamp{timestamp} {
}

std::string MetricEvent::getProgramName() const {
    return m_programName;
}

std::string MetricEvent::getSourceName() const {
    return m_sourceName;
}

const MetricContext& MetricEvent::getMetricContext() const {
    return m_metricContext;
}

DataPoint MetricEvent::getDataPoint(const std::string& name, DataType dataType) const {
    auto iter = m_dataPoints.find(name);
    if (iter == m_dataPoints.end()) {
        AACE_WARN(LX(TAG).m("requested data point doesn't exist").d("name", name));
        return DataPoint{"", "", DataType::COUNTER};
    }
    return iter->second;
}

std::vector<DataPoint> MetricEvent::getDataPoints() const {
    std::vector<DataPoint> dataPoints;
    for (const auto& entry : m_dataPoints) {
        dataPoints.push_back(entry.second);
    }
    return dataPoints;
}

std::chrono::system_clock::time_point MetricEvent::getSystemClockTimestamp() const {
    return std::chrono::system_clock::now() - std::chrono::duration_cast<std::chrono::system_clock::duration>(
                                                  std::chrono::steady_clock::now() - m_timestamp);
}

std::chrono::steady_clock::time_point MetricEvent::getSteadyClockTimestamp() const {
    return m_timestamp;
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
