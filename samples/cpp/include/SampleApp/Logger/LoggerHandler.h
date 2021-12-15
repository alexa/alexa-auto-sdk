/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_LOGGER_LOGGERHANDLER_H
#define SAMPLEAPP_LOGGER_LOGGERHANDLER_H

#include "SampleApp/Activity.h"

// C++ Standard Library
#include <chrono>

namespace sampleApp {

class ApplicationContext;  // forward declare

namespace logger {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LoggerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class LoggerHandler {
public:
    /**
     * Specifies the severity level of a log message
     */
    enum class Level {

        /**
         * Verbose log of an event, enabled only for debug builds
         */
        VERBOSE,

        /**
         * Log of a normal event. Used in release builds
         */
        INFO,

        /**
         * Log of a metric, enabled only for builds with metrics enabled
         */
        METRIC,

        /**
         * Log of an event that may indicate a problem
         */
        WARN,

        /**
         * Log of an event that indicates an error
         */
        ERROR,

        /**
         * Log of an event that indicates an unrecoverable error
         */
        CRITICAL
    };

protected:
    LoggerHandler(std::weak_ptr<Activity> activity);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<LoggerHandler> {
        return std::shared_ptr<LoggerHandler>(new LoggerHandler(args...));
    }

    void log(Level level, const std::string& tag, const std::string& message);

private:
    static std::string formatTime();
    std::weak_ptr<View> m_console{};
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LoggerHandler::Level
//
////////////////////////////////////////////////////////////////////////////////////////////////////

inline std::ostream& operator<<(std::ostream& stream, const LoggerHandler::Level& level) {
    using Level = LoggerHandler::Level;
    switch (level) {
        case Level::VERBOSE:
            stream << "VERBOSE";
            break;
        case Level::INFO:
            stream << "INFO";
            break;
        case Level::METRIC:
            stream << "METRIC";
            break;
        case Level::WARN:
            stream << "WARN";
            break;
        case Level::ERROR:
            stream << "ERROR";
            break;
        case Level::CRITICAL:
            stream << "CRITICAL";
            break;
    }
    return stream;
}

}  // namespace logger
}  // namespace sampleApp

#endif  // SAMPLEAPP_LOGGER_LOGGERHANDLER_H
