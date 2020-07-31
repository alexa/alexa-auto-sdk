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

#ifndef AACE_ENGINE_CORE_CORE_METRICS_H
#define AACE_ENGINE_CORE_CORE_METRICS_H

#include <string>

namespace aace {
namespace engine {
namespace core {

class CoreMetrics {
public:
    /**
     * Enum indicating the location where the metric message was issued
     */
    enum class Location {
        ENGINE_STOP_BEGIN,
        ENGINE_STOP_END,
        ENGINE_STOP_EXCEPTION,
        ENGINE_START_BEGIN,
        ENGINE_START_END,
        ENGINE_START_EXCEPTION
    };
};

inline std::ostream& operator<<(std::ostream& stream, const CoreMetrics::Location& location) {
    switch (location) {
        case CoreMetrics::Location::ENGINE_STOP_BEGIN:
            stream << "ENGINE_STOP_BEGIN";
            break;
        case CoreMetrics::Location::ENGINE_STOP_END:
            stream << "ENGINE_STOP_END";
            break;
        case CoreMetrics::Location::ENGINE_STOP_EXCEPTION:
            stream << "ENGINE_STOP_EXCEPTION";
            break;
        case CoreMetrics::Location::ENGINE_START_BEGIN:
            stream << "ENGINE_START_BEGIN";
            break;
        case CoreMetrics::Location::ENGINE_START_END:
            stream << "ENGINE_START_END";
            break;
        case CoreMetrics::Location::ENGINE_START_EXCEPTION:
            stream << "ENGINE_START_EXCEPTION";
            break;
    }
    return stream;
}

}  // namespace core
}  // namespace engine
}  // namespace aace

#ifdef AAC_LATENCY_LOGS_ENABLED
#define CORE_METRIC(entry, location) AACE_LOG(AACE_LOG_LEVEL::METRIC, entry.d("Location", location))
#else  // AAC_LATENCY_LOGS_ENABLED
#define CORE_METRIC(entry, location)
#endif  // AAC_LATENCY_LOGS_ENABLED

#endif  // AACE_ENGINE_CORE_CORE_METRICS_H
