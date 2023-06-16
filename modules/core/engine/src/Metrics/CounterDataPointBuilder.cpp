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

#include <limits>

#include <AACE/Engine/Core/EngineMacros.h>
#include "AACE/Engine/Metrics/CounterDataPointBuilder.h"

namespace aace {
namespace engine {
namespace metrics {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.CounterDataPointBuilder");

CounterDataPointBuilder::CounterDataPointBuilder() : m_value{0}, m_sampleCount{1} {
}

CounterDataPointBuilder& CounterDataPointBuilder::withName(const std::string& name) {
    m_name = name;
    return *this;
}

CounterDataPointBuilder& CounterDataPointBuilder::increment(uint64_t value) {
    if (m_value < (std::numeric_limits<uint64_t>::max() - value)) {
        m_value += value;
    } else {
        AACE_WARN(LX(TAG).m("Counter value overflow. Setting to max value"));
        m_value = std::numeric_limits<uint64_t>::max();
    }
    return *this;
}

CounterDataPointBuilder& CounterDataPointBuilder::withSampleCount(uint32_t count) {
    m_sampleCount = count;
    return *this;
}

DataPoint CounterDataPointBuilder::build() {
    return DataPoint{m_name, std::to_string(m_value), DataType::COUNTER, m_sampleCount};
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
