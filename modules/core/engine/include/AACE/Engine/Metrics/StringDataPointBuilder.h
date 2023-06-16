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

#ifndef AACE_ENGINE_METRICS_STRING_DATA_POINT_BUILDER_H
#define AACE_ENGINE_METRICS_STRING_DATA_POINT_BUILDER_H

#include <string>

#include <AACE/Engine/Metrics/DataPoint.h>

namespace aace {
namespace engine {
namespace metrics {

class StringDataPointBuilder {
public:
    /**
     * Sets the name of the data point.
     *
     * @param name The name of the data point.
     * @return The builder instance for chaining calls
     */
    StringDataPointBuilder& withName(const std::string& name);

    /**
     * Sets the value of the data point.
     *
     * @param name The value of the data point.
     * @return The builder instance for chaining calls
     */
    StringDataPointBuilder& withValue(const std::string& value);

    /**
     * Builds a string @c DataPoint object with the current state of the
     * builder.
     *
     * @return The string @c DataPoint object
     */
    DataPoint build();

private:
    /// The name of the data point
    std::string m_name;

    /// The value of the data point
    std::string m_value;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_STRING_DATA_POINT_BUILDER_H
