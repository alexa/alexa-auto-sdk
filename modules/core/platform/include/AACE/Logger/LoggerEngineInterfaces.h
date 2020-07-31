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

#ifndef AACE_LOGGER_LOGGER_ENGINE_INTERFACES_H
#define AACE_LOGGER_LOGGER_ENGINE_INTERFACES_H

#include <string>
#include <iostream>

/** @file */

namespace aace {
namespace logger {

/**
 * LoggerEngineInterface
 */
class LoggerEngineInterface {
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

    virtual void log(Level level, const std::string& tag, const std::string& message) = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const LoggerEngineInterface::Level& level) {
    switch (level) {
        case LoggerEngineInterface::Level::VERBOSE:
            stream << "VERBOSE";
            break;
        case LoggerEngineInterface::Level::INFO:
            stream << "INFO";
            break;
        case LoggerEngineInterface::Level::METRIC:
            stream << "METRIC";
            break;
        case LoggerEngineInterface::Level::WARN:
            stream << "WARN";
            break;
        case LoggerEngineInterface::Level::ERROR:
            stream << "ERROR";
            break;
        case LoggerEngineInterface::Level::CRITICAL:
            stream << "CRITICAL";
            break;
    }
    return stream;
}

}  // namespace logger
}  // namespace aace

#endif  // AACE_LOGGER_LOGGER_ENGINE_INTERFACES_H
