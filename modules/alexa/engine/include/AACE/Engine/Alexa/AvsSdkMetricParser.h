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

#ifndef AACE_ENGINE_ALEXA_AVS_SDK_METRIC_PARSER_H
#define AACE_ENGINE_ALEXA_AVS_SDK_METRIC_PARSER_H

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <AVSCommon/Utils/Metrics/DataPoint.h>
#include <AVSCommon/Utils/Metrics/MetricEvent.h>

#include <AACE/Engine/Metrics/DataPoint.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>

using namespace aace::engine::metrics;

namespace aace {
namespace engine {
namespace alexa {

enum class DataPointTransformType {
    INSERT_DIMENSION,
    SWAP_NAME,
    SPLIT_NAME,
    DROP_METRIC_IF_COUNT_ZERO,
    SWAP_NAME_OR_DROP_METRIC_IF_COUNT_ZERO,
    RENAME_COUNTERS
};

using DataPointTransformation = std::pair<DataPointTransformType, std::vector<std::string>>;

struct AvsSdkMetricTransformation {
    std::unordered_map<std::string, DataPointTransformation> namedDataPoints;
    std::vector<DataPointTransformation> allCounters;
    std::vector<DataPointTransformation> allStrings;    // currently unused
    std::vector<DataPointTransformation> allDurations;  // currently unused
};

class AvsSdkMetricParser {
public:
    /**
     * Convert the AVS SDK metric event to an Auto SDK @c MetricEvent. The
     * conversion involves applying any transformation rules to put the metric's
     * data points in Auto SDK's desired format.
     * 
     * @param metricEvent
     * @return A pointer to the converted metric or @c nullptr if the metric
     *         is intentionally dropped or has unexpected parse errors preventing
     *         conversion.
    */
    std::shared_ptr<MetricEvent> convertMetric(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricEvent> metricEvent);

    /**
     * Configure the parser with the rules specified in @c AvsSdkMetricRules.h.
     * @return @c true if parsing the rules succeeded, @c false if failed
     */
    bool configure();

    static bool convertWithoutChanges(
        MetricEventBuilder& builder,
        const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp);
    static bool insertDimension(
        MetricEventBuilder& builder,
        const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
        const std::vector<std::string>& args);
    static bool swapName(
        MetricEventBuilder& builder,
        const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
        const std::vector<std::string>& args);
    static bool splitName(
        MetricEventBuilder& builder,
        const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
        const std::vector<std::string>& args);
    static bool renameCounters(
        MetricEventBuilder& builder,
        const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
        const std::vector<std::string>& args);
    static bool dropMetricIfCountZero(
        MetricEventBuilder& builder,
        bool& drop,
        const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp);
    static bool swapNameOrDropMetricIfCountZero(
        MetricEventBuilder& builder,
        bool& drop,
        const alexaClientSDK::avsCommon::utils::metrics::DataPoint& sourceDp,
        const std::vector<std::string>& args);

private:
    /**
     * Any metric with a source whose prefix is in this set will be allowed and
     * converted with all data points as-is.
     */
    std::set<std::string> m_prefixOverrides;

    /**
     * Contains the allowed metric events keyed by source name. The value
     * is null if the metric data points are to be translated as-is. Nonnull
     * value specifies transformation rules to apply to the data points.
     */
    std::unordered_map<std::string, std::unique_ptr<AvsSdkMetricTransformation>> m_allowedSources;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_AVS_SDK_METRIC_PARSER_H
