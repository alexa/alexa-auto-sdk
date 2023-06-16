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
#include "AACE/Engine/Metrics/DurationDataPointBuilder.h"

namespace aace {
namespace engine {
namespace metrics {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.DurationDataPointBuilder");

DurationDataPointBuilder::DurationDataPointBuilder() : DurationDataPointBuilder(std::chrono::milliseconds(0)) {
}

DurationDataPointBuilder::DurationDataPointBuilder(std::chrono::milliseconds duration) :
        m_startTime{std::chrono::steady_clock::now()}, m_isTimerRunning{false} {
    std::chrono::milliseconds zeroMs(0);
    if (duration < zeroMs) {
        AACE_WARN(LX(TAG).m("Duration cannot be negative failed. Using 0ms"));
        duration = zeroMs;
    }
    m_duration = duration;
}

DurationDataPointBuilder& DurationDataPointBuilder::withName(const std::string& name) {
    m_name = name;
    return *this;
}

DurationDataPointBuilder& DurationDataPointBuilder::startTimer() {
    m_startTime = std::chrono::steady_clock::now();
    m_isTimerRunning = true;
    return *this;
}

DurationDataPointBuilder& DurationDataPointBuilder::stopTimer() {
    if (!m_isTimerRunning) {
        AACE_DEBUG(LX("stopTimer called while timer is not running"));
        return *this;
    }
    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
    m_duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime);
    m_isTimerRunning = false;
    return *this;
}

DataPoint DurationDataPointBuilder::build() {
    if (m_isTimerRunning) {
        AACE_WARN(LX("build() called while timer was still running. Stopping now"));
        stopTimer();
    }
    return DataPoint{m_name, std::to_string(m_duration.count()), DataType::DURATION};
}

}  // namespace metrics
}  // namespace engine
}  // namespace aace
