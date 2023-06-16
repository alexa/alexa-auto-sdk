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
#include <regex>
#include <string>
#include <sstream>

#include <nlohmann/json.hpp>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/AASBMetricsUtils.h>
#include <AACE/Engine/Metrics/DataPoint.h>
#include <AACE/Engine/Metrics/MetricContext.h>
#include <AACE/Engine/Metrics/MetricsConstants.h>
#include <AACE/Engine/Utils/Timing/ClockUtils.h>

using json = nlohmann::json;

namespace aace {
namespace engine {
namespace metrics {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.AASBMetricsUtils");

/// Regex to match data points in the AASB-serialized metric
static const std::regex DATA_REGEX("([^;=,:]+)=([^;=,]+);([^;=,:]+);([0-9]+),");

std::string serializeMetricEvent(const MetricEvent& metricEvent) {
    auto time_ms = aace::engine::utils::timing::timePointMillisSinceEpoch(metricEvent.getSystemClockTimestamp());
    const std::string timestamp = std::to_string(time_ms);

    const MetricContext& context = metricEvent.getMetricContext();
    const std::string agentId = std::to_string(context.getAgentId());
    const std::string priority = priorityToShortString(context.getPriority());
    const std::string identity = identityTypeToShortString(context.getIdentityType());
    const std::string program = metricEvent.getProgramName();
    const std::string source = metricEvent.getSourceName();

    const auto& dataPoints = metricEvent.getDataPoints();
    const std::string numDataPoints = std::to_string(dataPoints.size());
    std::stringstream ss_dp;
    for (const auto& dp : dataPoints) {
        const std::string name = dp.getName();
        const std::string value = dp.getValue();
        const std::string type = dataTypeToShortString(dp.getDataType());
        const std::string samples = std::to_string(dp.getSampleCount());
        // clang-format off
        ss_dp
            << name << EQUALS
            << value << SEMICOLON
            << type << SEMICOLON
            << samples << COMMA;
        // clang-format on
    }

    // clang-format off
    std::stringstream metric;
    metric
        << timestamp << COLON
        << agentId << COLON
        << priority << COLON
        << identity << COLON
        << program << COLON
        << source << COLON
        << numDataPoints << COLON
        << ss_dp.str();
    // clang-format on
    return metric.str();
}

size_t parseHeader(
    const std::string& aasbMetric,
    std::string& timestamp,
    std::string& agentId,
    std::string& priority,
    std::string& identityType,
    std::string& programName,
    std::string& sourceName,
    std::string& dpCount) {
    size_t pos = 0;

    for (int count = 0; count < 7; count++) {
        auto next = aasbMetric.find(":", pos);
        if (next == std::string::npos) {
            AACE_ERROR(LX(TAG).m("Missing ':'' delimiter before header entry"));
            return std::string::npos;
        }
        auto field = aasbMetric.substr(pos, next - pos);

        switch (count) {
            case 0:
                timestamp = field;
                break;
            case 1:
                agentId = field;
                break;
            case 2:
                priority = field;
                break;
            case 3:
                identityType = field;
                break;
            case 4:
                programName = field;
                break;
            case 5:
                sourceName = field;
                break;
            case 6:
                dpCount = field;
                break;
        }
        pos = next + 1;
    }
    return pos;
}

bool parseSerializedMetric(std::string aasbMetric, ParsedHeaderHandler headerHandler, DataPointAdder dpHandler) {
    std::string timestampStr;
    std::string agentId;
    std::string priority;
    std::string identityType;
    std::string programName;
    std::string sourceName;
    std::string dpCount;
    size_t dataPointPos =
        parseHeader(aasbMetric, timestampStr, agentId, priority, identityType, programName, sourceName, dpCount);
    if (dataPointPos == std::string::npos) {
        return false;
    }
    try {
        ThrowIf(timestampStr.empty(), "Failed to extract timestamp from serialized metric header");
        ThrowIf(agentId.empty(), "Failed to extract agent ID from serialized metric header");
        ThrowIf(priority.empty(), "Failed to extract priority from serialized metric header");
        ThrowIf(identityType.empty(), "Failed to extract identity type from serialized metric header");
        ThrowIf(programName.empty(), "Failed to extract program name from serialized metric header");
        ThrowIf(sourceName.empty(), "Failed to extract source name from serialized metric header");
        ThrowIf(dpCount.empty(), "Failed to extract data point count from serialized metric header");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).m("Parse metric header values failed").d("reason", ex.what()));
        return false;
    }
    headerHandler({timestampStr, agentId, priority, identityType, programName, sourceName, dpCount});

    auto dataPointsBegin = aasbMetric.cbegin() + (ssize_t)dataPointPos;
    auto dataPointsEnd = aasbMetric.cend();
    std::smatch data_match;
    try {
        while (std::regex_search(dataPointsBegin, dataPointsEnd, data_match, DATA_REGEX)) {
            ThrowIf(data_match.empty(), "Regex match is empty");
            const std::string name = data_match[1].str();
            ThrowIf(name.empty(), "Matched name is empty");
            const std::string value = data_match[2].str();
            ThrowIf(value.empty(), "Matched value is empty");
            const std::string typeStr = data_match[3].str();
            ThrowIf(typeStr.empty(), "Matched type empty");
            const std::string samplesStr = data_match[4];
            ThrowIf(samplesStr.empty(), "Matched sample count is empty");
            uint32_t sampleCount = static_cast<uint32_t>(std::stoi(samplesStr));
            dataPointsBegin = data_match.suffix().first;
            dpHandler(name, value, typeStr, sampleCount);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).m("Parse data point values failed").d("reason", ex.what()));
        return false;
    }
    return true;
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
