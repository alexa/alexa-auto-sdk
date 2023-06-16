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

#ifndef AACE_ENGINE_UTILS_TIMING_CLOCK_UTILS_H
#define AACE_ENGINE_UTILS_TIMING_CLOCK_UTILS_H

#include <chrono>

namespace aace {
namespace engine {
namespace utils {
namespace timing {

/**
 * @return The specified time as a count of milliseconds since epoch
 */
inline uint64_t timePointMillisSinceEpoch(const std::chrono::system_clock::time_point& time) {
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch());
    return static_cast<uint64_t>(now_ms.count());
}

/**
 * @return The current time as a count of milliseconds since epoch
 */
inline uint64_t currentTimeMillisSinceEpoch() {
    return timePointMillisSinceEpoch(std::chrono::system_clock::now());
}

}  // namespace timing
}  // namespace utils
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_UTILS_TIMING_CLOCK_UTILS_H
