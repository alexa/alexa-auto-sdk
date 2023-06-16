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

#include <AACE/Engine/Core/EngineMacros.h>
#include "AACE/Engine/Metrics/StringDataPointBuilder.h"

namespace aace {
namespace engine {
namespace metrics {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.StringDataPointBuilder");

StringDataPointBuilder& StringDataPointBuilder::withName(const std::string& name) {
    m_name = name;
    return *this;
}

StringDataPointBuilder& StringDataPointBuilder::withValue(const std::string& value) {
    m_value = value;
    return *this;
}

DataPoint StringDataPointBuilder::build() {
    return DataPoint{m_name, m_value, DataType::STRING};
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
