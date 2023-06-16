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

#ifndef AACE_ENGINE_METRICS_DURATION_DATA_POINT_BUILDER_H
#define AACE_ENGINE_METRICS_DURATION_DATA_POINT_BUILDER_H

#include <chrono>
#include <string>

#include <AACE/Engine/Metrics/DataPoint.h>

namespace aace {
namespace engine {
namespace metrics {

class DurationDataPointBuilder {
public:
    /**
     * Constructor.
     */
    DurationDataPointBuilder();

    /**
     * Constructor for a data point in which the duration is already known.
     *
     * @param duration The duration of the timer. Durations cannot be negative.
     */
    explicit DurationDataPointBuilder(std::chrono::milliseconds duration);

    /**
     * Sets the name of the data point.
     *
     * @param name The name of the data point.
     * @return The builder instance for chaining calls
     */
    DurationDataPointBuilder& withName(const std::string& name);

    /**
     * Starts a timer to calculate duration.
     * If @c startTimer is called more than once, subsequent calls will
     * overwrite the current timer with a new one.
     *
     * @return The builder instance for chaining calls
     */
    DurationDataPointBuilder& startTimer();

    /**
     * Stop the ongoing timer and maintain the calculated duration since
     * @c startTimer was called.
     * Calling @c stopTimer when the timer is not running is a no-op.
     *
     * @return The builder instance for chaining calls
     */
    DurationDataPointBuilder& stopTimer();

    /**
     * Builds a duration @c DataPoint object with the current state of the
     * builder. @c build() may be called while the timer is still running; the
     * builder will use the current duration.
     *
     * @return The counter @c DataPoint object
     */
    DataPoint build();

private:
    /// The name of the data point
    std::string m_name;

    /// The current duration
    std::chrono::milliseconds m_duration;

    /// The duration start time
    std::chrono::steady_clock::time_point m_startTime;

    /// Flag to indicate if the timer is running
    bool m_isTimerRunning;
};

}  // namespace metrics
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_METRICS_DURATION_DATA_POINT_BUILDER_H
