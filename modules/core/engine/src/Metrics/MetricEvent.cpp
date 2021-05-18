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

#include "AACE/Engine/Metrics/MetricEvent.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace metrics {

static const std::string TAG("MetricEvent");

/// Default number of samples for metric.
const std::string METRIC_NUM_SAMPLES_DEFAULT = "1";

MetricEvent::MetricEvent(const std::string& program, const std::string& source) :
        MetricEvent(program, source, MetricPriority::NR, MetricBufferType::NB, MetricIdentityType::NUNI) {
}

MetricEvent::MetricEvent(const std::string& program, const std::string& source, MetricPriority priority) :
        MetricEvent(program, source, priority, MetricBufferType::NB, MetricIdentityType::NUNI) {
}

MetricEvent::MetricEvent(const std::string& program, const std::string& source, MetricBufferType bufferType) :
        MetricEvent(program, source, MetricPriority::NR, bufferType, MetricIdentityType::NUNI) {
}

MetricEvent::MetricEvent(
    const std::string& program,
    const std::string& source,
    MetricBufferType bufferType,
    MetricIdentityType identityType) :
        MetricEvent(program, source, MetricPriority::NR, bufferType, identityType) {
}

MetricEvent::MetricEvent(
    const std::string& program,
    const std::string& source,
    MetricPriority priority,
    MetricBufferType bufferType,
    MetricIdentityType identityType) :
        m_program{program},
        m_source(source),
        m_metricLog{""},
        m_priority{priority},
        m_bufferType{bufferType},
        m_identityType{identityType} {
    m_metricLog.append(m_program).append(":").append(m_source);
}

void MetricEvent::addTimer(const std::string& name, double value) {
    addDataToLog(name, std::to_string(value), MetricDataType::TI, METRIC_NUM_SAMPLES_DEFAULT);
}

void MetricEvent::addString(const std::string& name, const std::string& value) {
    addDataToLog(name, value, MetricDataType::DV, METRIC_NUM_SAMPLES_DEFAULT);
}

void MetricEvent::addCounter(const std::string& name, int value) {
    addDataToLog(name, std::to_string(value), MetricDataType::CT, METRIC_NUM_SAMPLES_DEFAULT);
}

void MetricEvent::record() {
    std::string priorityStr = priorityToString(m_priority);
    m_metricLog.append(":").append(priorityStr);
    m_metricLog.append(":").append(bufferTypeToString(m_bufferType));
    m_metricLog.append(":").append(identityTypeToString(m_identityType));
    AACE_METRIC(LX(TAG, m_metricLog));
}

void MetricEvent::addDataToLog(std::string name, std::string value, MetricDataType type, std::string sampleCount) {
    m_metricLog.append(":").append(name).append("=").append(value).append(";");
    m_metricLog.append(dataTypeToString(type)).append(";").append(sampleCount).append(",");
}

std::string MetricEvent::priorityToString(MetricPriority priority) {
    switch (priority) {
        case MetricPriority::NR:
            return "NR";
        case MetricPriority::HI:
            return "HI";
    }
    return "";
}

std::string MetricEvent::dataTypeToString(MetricDataType type) {
    switch (type) {
        case MetricDataType::TI:
            return "TI";
        case MetricDataType::DV:
            return "DV";
        case MetricDataType::CT:
            return "CT";
    }
    return "";
}

std::string MetricEvent::bufferTypeToString(MetricBufferType type) {
    switch (type) {
        case MetricBufferType::BF:
            return "BF";
        case MetricBufferType::NB:
        default:
            return "NB";
    }
}

std::string MetricEvent::identityTypeToString(MetricIdentityType type) {
    switch (type) {
        case MetricIdentityType::UNIQ:
            return "UNIQ";
        case MetricIdentityType::NUNI:
        default:
            return "NUNI";
    }
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
