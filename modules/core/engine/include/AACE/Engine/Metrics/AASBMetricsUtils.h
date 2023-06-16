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

#ifndef AACE_ENGINE_METRICS_AASB_METRICS_UTILS_H
#define AACE_ENGINE_METRICS_AASB_METRICS_UTILS_H

#include <functional>

#include <AACE/Engine/Metrics/MetricContext.h>
#include <AACE/Engine/Metrics/MetricEvent.h>

namespace aace {
namespace engine {
namespace metrics {

using ParsedHeaderHandler = std::function<void(const std::vector<std::string>&)>;
using DataPointAdder = std::function<void(const std::string, const std::string, const std::string, uint32_t)>;

/** 
 * Returns the serialized representation of the specified @c MetricEvent, as
 * defined for a single metric in the "metrics" array of the
 * @c MetricsUpload.Agent<ID> AASB message.
 * 
 * @param metricEvent The metric to serialized
 * @return The serialized metric as a string 
 */
std::string serializeMetricEvent(const MetricEvent& metricEvent);

/**
 * Parse header values from the specified metric. The metric string must be
 * the serialized representation as defined for a single metric in the
 * "metrics" array of the @c MetricsUpload.Agent<ID> AASB message.
 * 
 * @param [in] aasbMetric The serialized metric as a string
 * @param [out] timestamp Reference for this function to write the timestamp
 *        parsed from the metric 
 * @param [out] agentId Reference for this function to write the agent ID
 *        parsed from the metric 
 * @param [out] priority Reference for this function to write the priority
 *        parsed from the metric
 * @param [out] identityType Reference for this function to write the identity
 *        type parsed from the metric
 * @param [out] programName Reference for this function to write the program
 *        name parsed from the metric
 * @param [out] sourceName Reference for this function to write the source
 *        name parsed from the metric
 * @param [out] dpCount Reference for this function to write the number of
 *        data points that the metric contains.
 * @return The starting position of the first data point in the metric.
 *         @c std::string::npos if there is a parsing error.
 */
size_t parseHeader(
    const std::string& aasbMetric,
    std::string& timestamp,
    std::string& agentId,
    std::string& priority,
    std::string& identityType,
    std::string& programName,
    std::string& sourceName,
    std::string& dpCount);

/**
 * Parse values from the specified metric. The metric string must be the
 * serialized representation as defined for a single metric in the "metrics"
 * array of the @c MetricsUpload.Agent<ID> AASB message. This function calls
 * @a headerHandler to set the header values then @a dpHandler one time for
 * each data point in the metric. Parsing is complete with handlers invoked by
 * the time this function returns.
 * 
 * @param [in] aasbMetric The serialized metric as a string
 * @param [in] headerHandler A function to invoke to set the header values
 *             parsed from the @a aasbMetric. The values in the vector parameter
 *             of @a headerHandler appear as follows: timestamp, agent ID,
 *             priority, identity type, program name, source name, data point
 *             count.
 * @param [in] dpHandler A function to invoke to set the values of a single
 *             data point parsed from @a aasbMetric. The parameters of
 *             @a dpHandler are data point name, value, type, and sample count.
 * @return @c true if @a aasbMetric was parsed properly and @a headerHandler
 *         and @a dpHandler were invoked with valid values. @c false if there
 *         was a parsing error
 */
bool parseSerializedMetric(std::string aasbMetric, ParsedHeaderHandler headerHandler, DataPointAdder dpHandler);

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_AASB_METRICS_UTILS_H
