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

#include "AACE/Engine/Metrics/MetricsUploaderEngineImpl.h"
#include "AACE/Engine/Logger/EngineLogger.h"
#include "AACE/Engine/Core/EngineMacros.h"

// String to identify log entries originating from this file.
static const std::string TAG("aace.metrics.MetricsUploaderEngineImpl");

namespace aace {
namespace engine {
namespace metrics {

const std::string MetricsUploaderEngineImpl::METRIC_RECORD_KEYWORD = "MetricEvent";
const std::string MetricsUploaderEngineImpl::PRIORITY_KEY = "Priority";
const std::string MetricsUploaderEngineImpl::PROGRAM_KEY = "Program";
const std::string MetricsUploaderEngineImpl::SOURCE_KEY = "Source";
const std::string MetricsUploaderEngineImpl::TIMER_KEY = "TI";
const std::string MetricsUploaderEngineImpl::STRING_KEY = "DV";
const std::string MetricsUploaderEngineImpl::COUNTER_KEY = "CT";

const std::string MetricsUploaderEngineImpl::NORMAL_PRIORITY = "NR";
const std::string MetricsUploaderEngineImpl::HIGH_PRIORITY = "HI";

static const std::regex dataRegex("([^;=,:]+)=([^;=,:]+);([^;=,:]+);([0-9]+),");
static const std::regex priorityRegex(":(NR|HI)");

// Helper function to parse metric header for programName and sourceName
size_t parseHeader(const std::string& metric, std::string& programName, std::string& sourceName) {
    size_t pos = 0;

    for (int count = 0; count < 3; count++) {
        auto next = metric.find(":", pos);
        if (next == std::string::npos) return std::string::npos;
        auto field = metric.substr(pos, next - pos);

        //Extract Program and source name
        switch (count) {
            case 1:
                programName = field;
                break;
            case 2:
                sourceName = field;
                break;
        }
        pos = next + 1;
    }
    return pos;
}

MetricsUploaderEngineImpl::MetricsUploaderEngineImpl(
    std::shared_ptr<aace::metrics::MetricsUploader> platformMetricsUploaderInterface) :
        aace::engine::logger::sink::Sink(TAG), m_platformMetricsUploaderInterface(platformMetricsUploaderInterface) {
}

std::shared_ptr<MetricsUploaderEngineImpl> MetricsUploaderEngineImpl::create(
    std::shared_ptr<aace::metrics::MetricsUploader> platformMetricsUploaderInterface) {
    try {
        ThrowIfNull(platformMetricsUploaderInterface, "invalidMetricsUploaderPlatformInterface");
        std::shared_ptr<MetricsUploaderEngineImpl> metricsUploaderEngineImpl =
            std::shared_ptr<MetricsUploaderEngineImpl>(new MetricsUploaderEngineImpl(platformMetricsUploaderInterface));

        ThrowIfNot(metricsUploaderEngineImpl->initialize(), "inializeMetricsUploaderEngineImplFailed");

        return metricsUploaderEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

bool MetricsUploaderEngineImpl::initialize() {
    try {
        ThrowIfNot(
            addRule(
                Level::METRIC,
                aace::engine::logger::sink::Rule::EMPTY,
                aace::engine::logger::sink::Rule::EMPTY,
                aace::engine::logger::sink::Rule::EMPTY),
            "addRuleFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

// ---------------------------------------------------------------------------
// DO NOT USE AACE_ MACROS TO LOG IN THIS FUNCTION AS IT WILL LEAD TO DEADLOCK
// ---------------------------------------------------------------------------
// aace::engine::logger::sink::Sink
void MetricsUploaderEngineImpl::log(
    Level level,
    std::chrono::system_clock::time_point time,
    const char* threadMoniker,
    const char* text) {
    try {
        //If log is not a Metric log, then return
        if (level != Level::METRIC) {
            return;
        }
        std::string logMessage = std::string(text);
        logMessage.erase(std::remove(logMessage.begin(), logMessage.end(), '\\'), logMessage.end());

        //If Metric log is not Recording related, then return
        if (logMessage.find(METRIC_RECORD_KEYWORD) == std::string::npos) {
            return;
        }

        //Parse program, source, datapoints to record metric
        std::string programName;
        std::string sourceName;
        size_t dataPointSepPos = parseHeader(logMessage, programName, sourceName);
        if (dataPointSepPos == std::string::npos) {
            return;
        }

        // Delimitate priority from metric log message by last 3 character of the message
        std::string metricPriority = logMessage.substr(logMessage.size() - 3, 3);
        std::smatch metricPriorityMatch;
        if (std::regex_match(metricPriority, metricPriorityMatch, priorityRegex)) {
            //Handle datapoints string by delimitate log message from datapoint separate position to metric priority
            std::string datapoints = logMessage.substr(dataPointSepPos, logMessage.size() - dataPointSepPos - 3);
            //Handle priority string by delimitate last 2 charaters from log message
            std::string priority = logMessage.substr(logMessage.size() - 2, 2);

            //Validate values are not empty/null
            if (programName.empty() || sourceName.empty() || datapoints.empty() || priority.empty()) {
                return;
            }

            //Create metadata map
            std::unordered_map<std::string, std::string> metadata;
            metadata[PROGRAM_KEY] = programName;
            metadata[SOURCE_KEY] = sourceName;
            metadata[PRIORITY_KEY] = priority;

            //Parse each datapoint and add to vector to pass to platform implementation
            std::vector<aace::metrics::MetricsUploader::Datapoint> datapointList;
            std::smatch data_match;
            while (std::regex_search(datapoints, data_match, dataRegex)) {
                //Handle regex groups
                std::string name = data_match[1].str();
                std::string value = data_match[2].str();
                std::string typeStr = data_match[3].str();
                std::string countStr = data_match[4].str();

                //Define dataType
                aace::metrics::MetricsUploader::DatapointType dataType;
                if (typeStr == TIMER_KEY) {
                    dataType = aace::metrics::MetricsUploader::DatapointType::TIMER;
                } else if (typeStr == STRING_KEY) {
                    dataType = aace::metrics::MetricsUploader::DatapointType::STRING;
                } else if (typeStr == COUNTER_KEY) {
                    dataType = aace::metrics::MetricsUploader::DatapointType::COUNTER;
                } else {
                    //No valid datatype was found
                    return;
                }

                //Create and add datapoint to list
                aace::metrics::MetricsUploader::Datapoint curData =
                    aace::metrics::MetricsUploader::Datapoint(dataType, name, value, stoi(countStr));
                datapointList.push_back(curData);

                //Set datapoints string equal to next datapoint for parsing until all datapoints parsed
                datapoints = data_match.suffix();
            }
            m_platformMetricsUploaderInterface->record(datapointList, metadata);
        }
    } catch (std::exception& ex) {
        //Exception occurred
    }
}
}  // namespace metrics
}  // namespace engine
}  // namespace aace
