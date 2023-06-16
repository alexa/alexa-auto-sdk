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

#include <chrono>
#include <ctime>
#include <istream>
#include <sstream>

#include <nlohmann/json.hpp>

#include <AACE/Engine/Alexa/AvsSdkMetricParser.h>
#include <AACE/Engine/Alexa/AvsSdkMetricRules.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Metrics/CounterDataPointBuilder.h>
#include <AACE/Engine/Metrics/DurationDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>
#include <AACE/Engine/Metrics/StringDataPointBuilder.h>

namespace aace {
namespace engine {
namespace alexa {

using json = nlohmann::json;
using namespace aace::engine::metrics;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AvsSdkMetricParser");

/// Program Name for the metrics emitted by Auto SDK
static const std::string PROGRAM_NAME = "AlexaAutoSDK";

DataType convertDataType(alexaClientSDK::avsCommon::utils::metrics::DataType type) {
    switch (type) {
        case alexaClientSDK::avsCommon::utils::metrics::DataType::DURATION:
            return DataType::DURATION;
        case alexaClientSDK::avsCommon::utils::metrics::DataType::COUNTER:
            return DataType::COUNTER;
        case alexaClientSDK::avsCommon::utils::metrics::DataType::STRING:
            return DataType::STRING;
        default:
            throw std::invalid_argument("invalid data type");
    }
}

std::shared_ptr<MetricEvent> AvsSdkMetricParser::convertMetric(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricEvent> metricEvent) {
    ThrowIfNull(metricEvent, "MetricEvent is null");
    const std::string activityName = metricEvent->getActivityName();
    bool useAsIs = false;

    for (const std::string& prefix : m_prefixOverrides) {
        if (activityName.rfind(prefix, 0) == 0) {
            AACE_VERBOSE(LX(TAG).m("Metric used without changes").d("source", activityName));
            useAsIs = true;
            break;
        }
    }
    auto sourceItr = m_allowedSources.find(activityName);
    if (!useAsIs) {
        if (sourceItr == m_allowedSources.end()) {
            AACE_VERBOSE(LX(TAG).m("Dropping unused metric").d("source", activityName));
            return nullptr;
        }
        if (sourceItr->second == nullptr) {
            useAsIs = true;
            AACE_VERBOSE(LX(TAG).m("Metric used without changes").d("source", activityName));
        }
    }

    auto metricBuilder = MetricEventBuilder().withSourceName(activityName);
    if (metricEvent->getPriority() == alexaClientSDK::avsCommon::utils::metrics::Priority::HIGH) {
        metricBuilder.withPriority(Priority::HIGH);
    }
    metricBuilder.withTimeStamp(metricEvent->getSteadyTimestamp());
    metricBuilder.withAgentId(metricEvent->getMetricContext().agentId);

    for (const auto& datapoint : metricEvent->getDataPoints()) {
        if (datapoint.isValid()) {
            auto datatype = datapoint.getDataType();
            auto name = datapoint.getName();
            auto value = datapoint.getValue();

            if (sourceItr != m_allowedSources.end() && sourceItr->second != nullptr) {
                bool success = false;
                AvsSdkMetricTransformation transformations = *(sourceItr->second);
                auto iter = transformations.namedDataPoints.find(name);
                bool dpHasNamedTransformation = iter != transformations.namedDataPoints.end();
                bool hasAllCounterTransformation = !transformations.allCounters.empty();
                if (dpHasNamedTransformation) {
                    const DataPointTransformation transform = iter->second;
                    DataPointTransformType type = transform.first;
                    const std::vector<std::string>& args = transform.second;
                    switch (type) {
                        case DataPointTransformType::INSERT_DIMENSION:
                            success = insertDimension(metricBuilder, datapoint, args);
                            break;
                        case DataPointTransformType::SWAP_NAME:
                            success = swapName(metricBuilder, datapoint, args);
                            break;
                        case DataPointTransformType::SPLIT_NAME:
                            success = splitName(metricBuilder, datapoint, args);
                            break;
                        case DataPointTransformType::DROP_METRIC_IF_COUNT_ZERO: {
                            bool drop = false;
                            success = dropMetricIfCountZero(metricBuilder, drop, datapoint);
                            if (drop) {
                                return nullptr;
                            }
                        }
                        case DataPointTransformType::SWAP_NAME_OR_DROP_METRIC_IF_COUNT_ZERO: {
                            bool drop = false;
                            success = swapNameOrDropMetricIfCountZero(metricBuilder, drop, datapoint, args);
                            if (drop) {
                                return nullptr;
                            }
                        }
                        default:
                            continue;
                    }
                } else if (
                    hasAllCounterTransformation &&
                    datatype == alexaClientSDK::avsCommon::utils::metrics::DataType::DURATION) {
                    for (const DataPointTransformation& dpt : transformations.allCounters) {
                        const auto args = dpt.second;
                        if (dpt.first == DataPointTransformType::RENAME_COUNTERS) {
                            success = renameCounters(metricBuilder, datapoint, args);
                        }
                    }
                } else {
                    success = convertWithoutChanges(metricBuilder, datapoint);
                }
                if (!success) {
                    AACE_WARN(LX(TAG).m("Issue transforming data point").d("source", activityName).d("name", name));
                    continue;
                }
            } else {
                convertWithoutChanges(metricBuilder, datapoint);
            }
        } else {
            AACE_WARN(LX(TAG).m("Dropping invalid data point"));
        }
    }
    try {
        return std::make_shared<MetricEvent>(metricBuilder.build());
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "Build MetricEvent failed").d("reason", ex.what()));
        return nullptr;
    }
}

bool AvsSdkMetricParser::configure() {
    try {
        std::stringstream stream(AVS_SDK_METRIC_RULES);
        ThrowIfNot(stream.good(), "invalid stream");
        json j = json::parse(stream);
        if (j.contains("prefixOverride")) {
            json prefixArray = j.at("prefixOverride");
            ThrowIfNot(prefixArray.is_array(), "prefixOverride is not an array");
            for (auto& prefixItr : prefixArray.items()) {
                std::string prefix = prefixItr.value();
                m_prefixOverrides.emplace(prefix);
                AACE_VERBOSE(LX(TAG).m("All metrics with prefix used without changes").d("prefix", prefix));
            }
        }
        if (!j.contains("allowed")) {
            AACE_WARN(LX(TAG).m("No metrics in AVS SDK allowlist"));
        }
        json allowedJ = j.at("allowed");
        for (auto& allowedItemItr : allowedJ.items()) {
            const json object = allowedItemItr.value();
            if (!object.contains("source")) {
                AACE_WARN(LX(TAG).m("Entry in 'allowed' is missing key 'source'"));
                continue;
            }
            const std::string source = object.at("source");
            bool hasRules = object.contains("transformRules");
            if (!hasRules) {
                AACE_VERBOSE(LX(TAG).m("Metric used without changes").d("source", source));
                m_allowedSources.emplace(source, nullptr);
                continue;
            }
            json rules = object.at("transformRules");
            std::unique_ptr<AvsSdkMetricTransformation> transformations =
                std::unique_ptr<AvsSdkMetricTransformation>(new AvsSdkMetricTransformation());
            for (auto& ruleItr : rules.items()) {
                const json ruleObject = ruleItr.value();
                AACE_VERBOSE(LX(TAG).m("Metric used with changes").d("source", source).d("rule", ruleObject.dump()));
                if (ruleObject.contains("swapName")) {
                    const json args = ruleObject.at("swapName");
                    const std::string dpName = args[0];
                    const std::string asValueOf = args[1];
                    const std::string asDimension = args[2];
                    transformations->namedDataPoints.emplace(
                        dpName,
                        std::make_pair(
                            DataPointTransformType::SWAP_NAME, std::vector<std::string>{asValueOf, asDimension}));
                }
                if (ruleObject.contains("insertDimension")) {
                    const json args = ruleObject.at("insertDimension");
                    const std::string dpName = args[0];
                    const std::string asDimension = args[1];
                    transformations->namedDataPoints.emplace(
                        dpName,
                        std::make_pair(
                            DataPointTransformType::INSERT_DIMENSION, std::vector<std::string>{asDimension}));
                }
                if (ruleObject.contains("splitName")) {
                    const json args = ruleObject.at("splitName");
                    const std::string dpName = args[0];
                    const std::string delimeter = args[1];
                    const std::string asValueOf = args[2];
                    const std::string suffixAsDimension = args[3];
                    transformations->namedDataPoints.emplace(
                        dpName,
                        std::make_pair(
                            DataPointTransformType::SPLIT_NAME,
                            std::vector<std::string>{delimeter, asValueOf, suffixAsDimension}));
                }
                if (ruleObject.contains("renameCounters")) {
                    const json args = ruleObject.at("renameCounters");
                    const std::string newName = args[0];
                    const std::string asValueOf = args[1];
                    transformations->allCounters.push_back(
                        {DataPointTransformType::RENAME_COUNTERS, std::vector<std::string>{newName, asValueOf}});
                    continue;
                }
                if (ruleObject.contains("dropMetricIfCountZero")) {
                    const json args = ruleObject.at("dropMetricIfCountZero");
                    const std::string dpName = args[0];
                    transformations->namedDataPoints.emplace(
                        dpName,
                        std::make_pair(DataPointTransformType::DROP_METRIC_IF_COUNT_ZERO, std::vector<std::string>{}));
                }
                if (ruleObject.contains("swapNameOrDropMetricIfCountZero")) {
                    const json args = ruleObject.at("swapNameOrDropMetricIfCountZero");
                    const std::string dpName = args[0];
                    const std::string asValueOf = args[1];
                    const std::string asDimension = args[2];
                    transformations->namedDataPoints.emplace(
                        dpName,
                        std::make_pair(
                            DataPointTransformType::SWAP_NAME_OR_DROP_METRIC_IF_COUNT_ZERO,
                            std::vector<std::string>{asValueOf, asDimension}));
                }
            }
            m_allowedSources.emplace(std::make_pair(source, std::move(transformations)));
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "configure failed").d("reason", ex.what()));
        return false;
    }
}

bool AvsSdkMetricParser::convertWithoutChanges(
    MetricEventBuilder& builder,
    const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp) {
    try {
        DataType type = convertDataType(sourceDp.getDataType());
        auto name = sourceDp.getName();
        auto value = sourceDp.getValue();
        builder.addDataPoint(DataPoint(name, value, type));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "convertWithoutChanges failed").d("reason", ex.what()));
        return false;
    }
}

bool AvsSdkMetricParser::insertDimension(
    MetricEventBuilder& metricBuilder,
    const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
    const std::vector<std::string>& args) {
    try {
        const std::string asDimension = args[0];
        if (asDimension.empty()) {
            AACE_ERROR(LX(TAG).m("Invalid rules"));
            return false;
        }
        DataType type = convertDataType(sourceDp.getDataType());
        metricBuilder.addDataPoint(DataPoint(sourceDp.getName(), sourceDp.getValue(), type));
        metricBuilder.addDataPoint(DataPoint(asDimension, sourceDp.getName(), DataType::STRING));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "configure failed").d("reason", ex.what()));
        return false;
    }
}

bool AvsSdkMetricParser::swapName(
    MetricEventBuilder& metricBuilder,
    const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
    const std::vector<std::string>& args) {
    try {
        const std::string asValueOf = args[0];
        const std::string asDimension = args[1];
        if (asValueOf.empty() || asDimension.empty()) {
            AACE_ERROR(LX(TAG).m("Invalid rules for swapName"));
            return false;
        }
        DataType type = convertDataType(sourceDp.getDataType());
        metricBuilder.addDataPoint(DataPoint(asValueOf, sourceDp.getValue(), type));
        metricBuilder.addDataPoint(DataPoint(asDimension, sourceDp.getName(), DataType::STRING));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "Data point swapName failed").d("reason", ex.what()));
        return false;
    }
}

bool AvsSdkMetricParser::splitName(
    MetricEventBuilder& metricBuilder,
    const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
    const std::vector<std::string>& args) {
    try {
        const std::string delimeter = args[0];
        const std::string asValueOf = args[1];
        const std::string suffixAsDimension = args[2];
        if (delimeter.empty() || asValueOf.empty() || suffixAsDimension.empty()) {
            AACE_ERROR(LX(TAG).m("Invalid rules for splitName"));
            return false;
        }
        const std::string name = sourceDp.getName();
        auto pos = name.find(delimeter);
        if (pos == std::string::npos) {
            AACE_ERROR(
                LX(TAG).m("Could not find delimeter in data point name").d("delimeter", delimeter).d("name", name));
            return false;
        }
        std::string suffix = name.substr(pos + delimeter.length());
        if (suffix.empty()) {
            AACE_ERROR(LX(TAG).m("Empty suffix after delimeter").d("delimeter", delimeter).d("name", name));
            return false;
        }
        DataType type = convertDataType(sourceDp.getDataType());
        metricBuilder.addDataPoint(DataPoint(asValueOf, sourceDp.getValue(), type));
        metricBuilder.addDataPoint(DataPoint(suffixAsDimension, suffix, DataType::STRING));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "Data point splitName failed").d("reason", ex.what()));
        return false;
    }
}
bool AvsSdkMetricParser::renameCounters(
    MetricEventBuilder& metricBuilder,
    const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
    const std::vector<std::string>& args) {
    try {
        const std::string newName = args[0];
        const std::string asValueOf = args[1];
        if (newName.empty() || asValueOf.empty()) {
            AACE_ERROR(LX(TAG).m("Invalid rules for renameCounters"));
            return false;
        }
        const std::string name = sourceDp.getName();
        DataType type = convertDataType(sourceDp.getDataType());
        if (type != DataType::COUNTER) {
            return false;
        }
        metricBuilder.addDataPoint(DataPoint(newName, sourceDp.getValue(), type));
        metricBuilder.addDataPoint(DataPoint(asValueOf, name, DataType::STRING));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "Data point renameCounters failed").d("reason", ex.what()));
        return false;
    }
}

bool AvsSdkMetricParser::dropMetricIfCountZero(
    MetricEventBuilder& metricBuilder,
    bool& drop,
    const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp) {
    try {
        const std::string name = sourceDp.getName();
        DataType type = convertDataType(sourceDp.getDataType());
        if (type != DataType::COUNTER) {
            return false;
        }
        const std::string val = sourceDp.getValue();
        long count = std::stoul(val);
        if (count == 0) {
            drop = true;
            return true;
        }
        drop = false;
        metricBuilder.addDataPoint(DataPoint(name, val, type));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "Data point dropMetricIfCountZero failed").d("reason", ex.what()));
        return false;
    }
}

bool AvsSdkMetricParser::swapNameOrDropMetricIfCountZero(
    MetricEventBuilder& metricBuilder,
    bool& drop,
    const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
    const std::vector<std::string>& args) {
    try {
        const std::string name = sourceDp.getName();
        DataType type = convertDataType(sourceDp.getDataType());
        if (type != DataType::COUNTER) {
            return false;
        }
        const std::string val = sourceDp.getValue();
        long count = std::stoul(val);
        if (count == 0) {
            drop = true;
            return true;
        }
        drop = false;
        return swapName(metricBuilder, sourceDp, args);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "Data point swapNameOrDropMetricIfCountZero failed").d("reason", ex.what()));
        return false;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
