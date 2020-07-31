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

#include <sstream>

#include "AACE/Engine/Logger/LogFormatter.h"

namespace aace {
namespace engine {
namespace logger {

/// Format string for strftime() to produce date and time in the format "YYYY-MM-DD HH:MM:SS".
static const char* STRFTIME_FORMAT_STRING = "%Y-%m-%d %H:%M:%S";

/// Size of buffer needed to hold "YYYY-MM-DD HH:MM:SS" and a null terminator.
static const int DATE_AND_TIME_STRING_SIZE = 20;

/// Separator between date/time and milliseconds.
static const char TIME_AND_MILLIS_SEPARATOR = '.';

// Format string for sprintf() to produce milliseconds in the format "nnn".
static const char* MILLIS_FORMAT_STRING = "%03d";

/// Size of buffer needed to hold "nnn" (milliseconds value) and a null terminator
static const int MILLIS_STRING_SIZE = 4;

/// Separator string between milliseconds value and ExampleLogger name.
static const std::string MILLIS_AND_THREAD_SEPARATOR = " [";

/// Separator between thread ID and level indicator in log lines.
static const std::string THREAD_AND_LEVEL_SEPARATOR = "] ";

/// Separator between level indicator and text in log lines.
static const char LEVEL_AND_TEXT_SEPARATOR = ' ';

/// Number of milliseconds per second.
static const int MILLISECONDS_PER_SECOND = 1000;

std::string LogFormatter::format(
    Level level,
    std::chrono::system_clock::time_point time,
    const char* threadMoniker,
    const char* text) {
    std::stringstream stringToEmit;

    char levelCh;
    switch (level) {
        case Level::CRITICAL:
            levelCh = 'C';
            break;
        case Level::ERROR:
            levelCh = 'E';
            break;
        case Level::INFO:
            levelCh = 'I';
            break;
        case Level::VERBOSE:
            levelCh = 'V';
            break;
        case Level::WARN:
            levelCh = 'W';
            break;
        case Level::METRIC:
            levelCh = 'M';
            break;
        default:
            levelCh = '?';
            break;
    }

    if (time.time_since_epoch().count() > 0) {
        bool dateTimeFailure = false;
        bool millisecondFailure = false;
        char dateTimeString[DATE_AND_TIME_STRING_SIZE];
        auto timeAsTime_t = std::chrono::system_clock::to_time_t(time);
        auto timeAsTmPtr = std::gmtime(&timeAsTime_t);
        if (!timeAsTmPtr ||
            0 == strftime(dateTimeString, sizeof(dateTimeString), STRFTIME_FORMAT_STRING, timeAsTmPtr)) {
            dateTimeFailure = true;
        }
        auto timeMillisPart = static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count() %
            MILLISECONDS_PER_SECOND);
        char millisString[MILLIS_STRING_SIZE];
        if (std::snprintf(millisString, sizeof(millisString), MILLIS_FORMAT_STRING, timeMillisPart) < 0) {
            millisecondFailure = true;
        }

        stringToEmit << (dateTimeFailure ? "ERROR: strftime() failed.  Date and time not logged." : dateTimeString)
                     << TIME_AND_MILLIS_SEPARATOR
                     << (millisecondFailure ? "ERROR: snprintf() failed.  Milliseconds not logged." : millisString)
                     << MILLIS_AND_THREAD_SEPARATOR << threadMoniker << THREAD_AND_LEVEL_SEPARATOR << levelCh
                     << LEVEL_AND_TEXT_SEPARATOR << text;
    } else {
        stringToEmit << "[" << threadMoniker << THREAD_AND_LEVEL_SEPARATOR << levelCh << LEVEL_AND_TEXT_SEPARATOR
                     << text;
    }

    return stringToEmit.str();
}

}  // namespace logger
}  // namespace engine
}  // namespace aace
