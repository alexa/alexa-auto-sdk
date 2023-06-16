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

#ifndef AACE_ENGINE_METRICS_COUNTER_DATA_POINT_BUILDER_H
#define AACE_ENGINE_METRICS_COUNTER_DATA_POINT_BUILDER_H

#include <string>

#include <AACE/Engine/Metrics/DataPoint.h>

namespace aace {
namespace engine {
namespace metrics {

class CounterDataPointBuilder {
public:
    /**
     * Constructor.
     */
    CounterDataPointBuilder();

    /**
     * Sets the name of the data point.
     *
     * @param name The name of the data point.
     * @return The builder instance for chaining calls
     */
    CounterDataPointBuilder& withName(const std::string& name);

    /**
     * Increments the value of the counter.
     *
     * @param value The value to add to the counter
     * @return The builder instance for chaining calls
     */
    CounterDataPointBuilder& increment(uint64_t value);

    /**
     * Sets the number of samples that comprise the counter value.
     * The default count is 1 if this function is not called.
     *
     * @param count The number of samples comprising the count
     * @return The builder instance for chaining calls
     */
    CounterDataPointBuilder& withSampleCount(uint32_t count);

    /**
     * Builds a counter @c DataPoint object with the current state of the
     * builder.
     *
     * @return The counter @c DataPoint object
     */
    DataPoint build();

private:
    /// The name of the data point.
    std::string m_name;

    /// The value of the counter.
    uint64_t m_value;

    /// The number of samples comprising this counter.
    uint32_t m_sampleCount;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_COUNTER_DATA_POINT_BUILDER_H
