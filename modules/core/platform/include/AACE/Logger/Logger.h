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

#ifndef AACE_LOGGER_LOGGER_H
#define AACE_LOGGER_LOGGER_H

#include <string>
#include <chrono>

#include "AACE/Core/PlatformInterface.h"
#include "LoggerEngineInterfaces.h"

/** @file */

namespace aace {
namespace logger {

/**
 * Logger should be extended handle log events from the AAC SDK.
 */
class Logger : public aace::core::PlatformInterface {
protected:
    Logger() = default;

public:
    virtual ~Logger();

    /**
     * Specifies the severity level of a log message
     * @sa @c aace::logger::LoggerEngineInterface::Level
     */
    using Level = aace::logger::LoggerEngineInterface::Level;

    /**
     * Notifies the platform implementation of a log event from the AAC SDK logger
     *
     * @param [in] level The log level
     * @param [in] time The timestamp of the logged message
     * @param [in] source The source of the log message
     * @param [in] message The log message
     *
     * @return @c true if the platform implementation successfully handled the log event, else @c false
     */
    virtual bool logEvent(
        aace::logger::Logger::Level level,
        std::chrono::system_clock::time_point time,
        const std::string& source,
        const std::string& message);

    /**
     * Notifies the Engine to use the AAC SDK logger to log a message originating on the platform.
     * The log event will be received by the platform with a call to @c logEvent() from the Engine.
     *
     * @param [in] level The log level
     * @param [in] tag The log tag
     * @param [in] message The log message
     */
    void log(Level level, const std::string& tag, const std::string& message);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<aace::logger::LoggerEngineInterface> loggerEngineInterface);

private:
    std::shared_ptr<aace::logger::LoggerEngineInterface> m_loggerEngineInterface;
};

}  // namespace logger
}  // namespace aace

#endif  // AACE_LOGGER_LOGGER_H
