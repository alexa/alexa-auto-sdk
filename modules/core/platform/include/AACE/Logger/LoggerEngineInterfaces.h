/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

/** @file */

namespace aace {
namespace logger {

class LoggerEngineInterface {
public:
    /**
     * Enum used to specify the severity assigned to a log message.
     */
    enum class Level {
        /**
         * Most verbose log level. Only enabled for debug builds.
         */
        VERBOSE,
        /**
         * Logs of normal operations, to be used in release builds.
         */
        INFO,
        /**
         * Log of an event that may indicate a problem.
         */
        WARN,
        /**
         * Log of an event that indicates an error.
         */
        ERROR,
        /**
         * Log of an event that indicates an unrecoverable error.
         */
        CRITICAL
    };

    virtual void log( Level level, const std::string& tag, const std::string& message ) = 0;
};

} // aace::logger
} // aace

#endif // AACE_LOGGER_LOGGER_ENGINE_INTERFACES_H
