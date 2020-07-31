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

#ifndef AACE_ENGINE_ALEXA_ALEXA_METRICS_H
#define AACE_ENGINE_ALEXA_ALEXA_METRICS_H

#include <iostream>
#include <string>

namespace aace {
namespace engine {
namespace alexa {

class AlexaMetrics {
public:
    /**
     * Enum indicating the location where the metric message was issued
     */
    enum class Location {
        SPEECH_START_CAPTURE,
        SPEECH_STOP_CAPTURE,
        PLAYBACK_REQUEST_START,
        PLAYBACK_REQUEST_RESUME,
        PLAYBACK_STARTED,
        PLAYBACK_RESUMED,
        PLAYBACK_FINISHED,
        END_OF_SPEECH
    };
};

inline std::ostream& operator<<(std::ostream& stream, const AlexaMetrics::Location& location) {
    switch (location) {
        case AlexaMetrics::Location::SPEECH_START_CAPTURE:
            stream << "SPEECH_START_CAPTURE";
            break;
        case AlexaMetrics::Location::SPEECH_STOP_CAPTURE:
            stream << "SPEECH_STOP_CAPTURE";
            break;
        case AlexaMetrics::Location::PLAYBACK_REQUEST_START:
            stream << "PLAYBACK_REQUEST_START";
            break;
        case AlexaMetrics::Location::PLAYBACK_REQUEST_RESUME:
            stream << "PLAYBACK_REQUEST_RESUME";
            break;
        case AlexaMetrics::Location::PLAYBACK_STARTED:
            stream << "PLAYBACK_STARTED";
            break;
        case AlexaMetrics::Location::PLAYBACK_RESUMED:
            stream << "PLAYBACK_RESUMED";
            break;
        case AlexaMetrics::Location::PLAYBACK_FINISHED:
            stream << "PLAYBACK_FINISHED";
            break;
        case AlexaMetrics::Location::END_OF_SPEECH:
            stream << "END_OF_SPEECH";
            break;
    }
    return stream;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#ifdef AAC_LATENCY_LOGS_ENABLED
#define ALEXA_METRIC(entry, location) AACE_LOG(AACE_LOG_LEVEL::METRIC, entry.d("Location", location))
#else  // AAC_LATENCY_LOGS_ENABLED
#define ALEXA_METRIC(entry, location)
#endif  // AAC_LATENCY_LOGS_ENABLED

#endif  // AACE_ENGINE_ALEXA_ALEXA_METRICS_H
