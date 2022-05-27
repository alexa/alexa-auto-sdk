/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#define MILLIS_AND_THREAD_SEPARATOR "["

/// Separator between thread ID and level indicator in log lines.
#define THREAD_AND_LEVEL_SEPARATOR "]"

/// Separator between level indicator and text in log lines.
static const char LEVEL_AND_TEXT_SEPARATOR = ' ';

/// Number of milliseconds per second.
static const int MILLISECONDS_PER_SECOND = 1000;

char LogFormatter::getLevelCh(const LogFormatter::Level& level) const {
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
    return levelCh;
}

std::string LogFormatter::format(
    Level level,
    std::chrono::system_clock::time_point time,
    const char* source,
    const char* threadMoniker,
    const char* text) {
    std::stringstream stringToEmit;

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

        formatWithTime(
            stringToEmit,
            dateTimeFailure,
            dateTimeString,
            millisecondFailure,
            millisString,
            source,
            threadMoniker,
            level,
            text);
    } else {
        formatWithoutTime(stringToEmit, source, threadMoniker, level, text);
    }

    return stringToEmit.str();
}

class PlainTextLogFormatter : public LogFormatter {
public:
    void formatWithTime(
        std::stringstream& stringToEmit,
        bool dateTimeFailure,
        const char* dateTimeString,
        bool millisecondFailure,
        const char* millisString,
        const char* source,
        const char* threadMoniker,
        LogFormatter::Level level,
        const char* text) override;

    void formatWithoutTime(
        std::stringstream& stringToEmit,
        const char* source,
        const char* threadMoniker,
        LogFormatter::Level level,
        const char* text) override;
};

void PlainTextLogFormatter::formatWithTime(
    std::stringstream& stringToEmit,
    bool dateTimeFailure,
    const char* dateTimeString,
    bool millisecondFailure,
    const char* millisString,
    const char* source,
    const char* threadMoniker,
    LogFormatter::Level level,
    const char* text) {
    stringToEmit << (dateTimeFailure ? "ERROR: strftime() failed.  Date and time not logged." : dateTimeString)
                 << TIME_AND_MILLIS_SEPARATOR
                 << (millisecondFailure ? "ERROR: snprintf() failed.  Milliseconds not logged." : millisString)
                 << " " MILLIS_AND_THREAD_SEPARATOR << source << THREAD_AND_LEVEL_SEPARATOR
#ifdef AAC_EMIT_THREAD_MONIKER_LOGS
                 << MILLIS_AND_THREAD_SEPARATOR << threadMoniker << THREAD_AND_LEVEL_SEPARATOR
#endif
                 << " " << getLevelCh(level) << LEVEL_AND_TEXT_SEPARATOR << text;
}

void PlainTextLogFormatter::formatWithoutTime(
    std::stringstream& stringToEmit,
    const char* source,
    const char* threadMoniker,
    LogFormatter::Level level,
    const char* text) {
    stringToEmit << " " MILLIS_AND_THREAD_SEPARATOR << source << THREAD_AND_LEVEL_SEPARATOR
#ifdef AAC_EMIT_THREAD_MONIKER_LOGS
                 << MILLIS_AND_THREAD_SEPARATOR << threadMoniker << THREAD_AND_LEVEL_SEPARATOR
#endif
                 << " " << getLevelCh(level) << LEVEL_AND_TEXT_SEPARATOR << text;
}

std::unique_ptr<LogFormatter> LogFormatter::createPlainText() {
    return std::unique_ptr<LogFormatter>(new PlainTextLogFormatter());
}

enum class Color {
    RESET = 0,

    FG_BLACK = 30,
    FG_RED = 31,
    FG_GREEN = 32,
    FG_YELLOW = 33,
    FG_BLUE = 34,
    FG_MAGENTA = 35,
    FG_CYAN = 36,
    FG_LIGHT_GRAY = 37,
    FG_DEFAULT = 39,

    FG_DARK_GRAY = 90,
    FG_LIGHT_RED = 91,
    FG_LIGHT_GREEN = 92,
    FG_LIGHT_YELLOW = 93,
    FG_LIGHT_BLUE = 94,
    FG_LIGHT_MAGENTA = 95,
    FG_LIGHT_CYAN = 96,
    FG_WHITE = 97,
};

std::ostream& operator<<(std::ostream& os, Color c) {
    return os << "\033[" << (int)c << "m";
}

class ColorLogFormatter : public LogFormatter {
public:
    void formatWithTime(
        std::stringstream& stringToEmit,
        bool dateTimeFailure,
        const char* dateTimeString,
        bool millisecondFailure,
        const char* millisString,
        const char* source,
        const char* threadMoniker,
        LogFormatter::Level level,
        const char* text) override;

    void formatWithoutTime(
        std::stringstream& stringToEmit,
        const char* source,
        const char* threadMoniker,
        LogFormatter::Level level,
        const char* text) override;

private:
    static Color getLevelColor(const LogFormatter::Level& level);
};

Color ColorLogFormatter::getLevelColor(const LogFormatter::Level& level) {
    using Level = LogFormatter::Level;

    switch (level) {
        case Level::VERBOSE:
            return Color::FG_DARK_GRAY;
        case Level::INFO:
            return Color::FG_WHITE;
        case Level::METRIC:
            return Color::FG_LIGHT_BLUE;
        case Level::WARN:
            return Color::FG_YELLOW;
        case Level::ERROR:
            return Color::FG_LIGHT_RED;
        case Level::CRITICAL:
            return Color::FG_LIGHT_MAGENTA;
        default:
            return Color::FG_DEFAULT;
    }
}
void ColorLogFormatter::formatWithTime(
    std::stringstream& stringToEmit,
    bool dateTimeFailure,
    const char* dateTimeString,
    bool millisecondFailure,
    const char* millisString,
    const char* source,
    const char* threadMoniker,
    LogFormatter::Level level,
    const char* text) {
    stringToEmit << Color::FG_LIGHT_GRAY
                 << (dateTimeFailure ? "ERROR: strftime() failed.  Date and time not logged." : dateTimeString)
                 << TIME_AND_MILLIS_SEPARATOR
                 << (millisecondFailure ? "ERROR: snprintf() failed.  Milliseconds not logged." : millisString)
                 << Color::FG_LIGHT_CYAN << " " MILLIS_AND_THREAD_SEPARATOR << source << THREAD_AND_LEVEL_SEPARATOR
#ifdef AAC_EMIT_THREAD_MONIKER_LOGS
                 << Color::FG_LIGHT_GREEN << MILLIS_AND_THREAD_SEPARATOR << threadMoniker << THREAD_AND_LEVEL_SEPARATOR
#endif
                 << " " << getLevelColor(level) << getLevelCh(level) << LEVEL_AND_TEXT_SEPARATOR << text
                 << Color::FG_DEFAULT;
}

void ColorLogFormatter::formatWithoutTime(
    std::stringstream& stringToEmit,
    const char* source,
    const char* threadMoniker,
    LogFormatter::Level level,
    const char* text) {
    stringToEmit << Color::FG_LIGHT_CYAN << MILLIS_AND_THREAD_SEPARATOR << source << THREAD_AND_LEVEL_SEPARATOR
#ifdef AAC_EMIT_THREAD_MONIKER_LOGS
                 << Color::FG_LIGHT_GREEN << MILLIS_AND_THREAD_SEPARATOR << threadMoniker << THREAD_AND_LEVEL_SEPARATOR
#endif
                 << " " << getLevelColor(level) << getLevelCh(level) << LEVEL_AND_TEXT_SEPARATOR << text
                 << Color::FG_DEFAULT;
}

std::unique_ptr<LogFormatter> LogFormatter::createColor() {
    return std::unique_ptr<LogFormatter>(new ColorLogFormatter());
}

}  // namespace logger
}  // namespace engine
}  // namespace aace
