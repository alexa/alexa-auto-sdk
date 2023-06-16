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

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Metrics/DataPoint.h"

namespace aace {
namespace engine {
namespace metrics {

/// Tag to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.DataPoint");

DataPoint::DataPoint(const std::string& name, const std::string& value, DataType dataType, uint32_t sampleCount) :
        m_name{name}, m_value{value}, m_dataType{dataType}, m_sampleCount{sampleCount} {
}

std::string DataPoint::getName() const {
    return m_name;
}

std::string DataPoint::getValue() const {
    return m_value;
}

DataType DataPoint::getDataType() const {
    return m_dataType;
}

uint32_t DataPoint::getSampleCount() const {
    return m_sampleCount;
}

bool DataPoint::isValid() const {
    return !m_name.empty() && !m_value.empty() && m_sampleCount != 0;
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
