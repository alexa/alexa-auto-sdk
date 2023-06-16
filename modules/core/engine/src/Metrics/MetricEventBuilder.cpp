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

#include <AACE/Engine/Core/EngineMacros.h>
#include "AACE/Engine/Metrics/MetricEventBuilder.h"
#include <AACE/Engine/Utils/Agent/AgentId.h>

namespace aace {
namespace engine {
namespace metrics {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.MetricEventBuilder");

/// The default program name for Alexa Auto SDK metrics.
static const std::string AUTO_SDK_PROGRAM_NAME("AlexaAutoSDK");

MetricEventBuilder::MetricEventBuilder() :
        m_programName{AUTO_SDK_PROGRAM_NAME},
        m_sourceName{""},
        m_agentId{aace::engine::utils::agent::AGENT_ID_NONE},
        m_priority{Priority::NORMAL},
        m_bufferType{BufferType::NO_BUFFER},
        m_identityType{IdentityType::NORMAL},
        m_overloadTimestamp{false},
        m_timestamp{std::chrono::steady_clock::now()} {
}

MetricEventBuilder& MetricEventBuilder::withProgramName(const std::string& programName) {
    m_programName = programName;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::withSourceName(const std::string& sourceName) {
    m_sourceName = sourceName;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::withPriority(Priority priority) {
    m_priority = priority;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::withBufferType(BufferType bufferType) {
    m_bufferType = bufferType;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::withIdentityType(IdentityType identityType) {
    m_identityType = identityType;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::withAgentId(AgentIdType agentId) {
    m_agentId = agentId;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::withAlexaAgentId() {
    m_agentId = aace::engine::utils::agent::AGENT_ID_ALEXA;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::withAllAgentId() {
    m_agentId = aace::engine::utils::agent::AGENT_ID_ALL;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::withNoneAgentId() {
    m_agentId = aace::engine::utils::agent::AGENT_ID_NONE;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::withTimeStamp(std::chrono::steady_clock::time_point timestamp) {
    m_overloadTimestamp = true;
    m_timestamp = timestamp;
    return *this;
}

MetricEventBuilder& MetricEventBuilder::addMetadata(const std::string& key, const std::string& value) {
    m_metadata.insert({key, value});
    return *this;
}

MetricEventBuilder& MetricEventBuilder::addMetadata(std::unordered_map<std::string, std::string> metadata) {
    m_metadata.insert(metadata.begin(), metadata.end());
    return *this;
}

MetricEventBuilder& MetricEventBuilder::addDataPoint(const DataPoint& dataPoint) {
    if (!dataPoint.isValid()) {
        AACE_WARN(LX(TAG).m("addDataPoint failed. DataPoint is invalid"));
        return *this;
    }
    auto name = dataPoint.getName();
    if (m_dataPoints.find(name) != m_dataPoints.end()) {
        AACE_WARN(LX(TAG).m("DataPoint name already exists. Overwriting"));
        m_dataPoints.erase(name);
    }
    m_dataPoints.insert({name, dataPoint});
    return *this;
}

MetricEventBuilder& MetricEventBuilder::addDataPoints(const std::vector<DataPoint>& dataPoints) {
    for (const auto& dataPoint : dataPoints) {
        addDataPoint(dataPoint);
    }
    return *this;
}

MetricEvent MetricEventBuilder::build() {
    if (m_programName.empty() || m_sourceName.empty()) {
        AACE_ERROR(LX(TAG)
                       .m("Failed to build MetricEvent. Invalid program or source name")
                       .d("program", m_programName)
                       .d("source", m_sourceName));
        throw std::invalid_argument("Invalid program or source name");
    }
    if (m_dataPoints.size() == 0) {
        AACE_ERROR(LX(TAG).m("Failed to build MetricEvent. No data points"));
        throw std::invalid_argument("Metric has no data points");
    }
    auto timestamp = m_overloadTimestamp ? m_timestamp : std::chrono::steady_clock::now();
    MetricContext context(m_agentId, m_priority, m_bufferType, m_identityType, m_metadata);
    return MetricEvent(m_programName, m_sourceName, std::move(context), m_dataPoints, timestamp);
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
