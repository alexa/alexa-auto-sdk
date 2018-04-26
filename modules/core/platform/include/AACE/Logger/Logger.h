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
 * The @c Logger should be extended by the platform implementation to handle logs coming from the SDK.
 */
class Logger : public aace::core::PlatformInterface {
protected:
    Logger() = default;

public:
    virtual ~Logger() = default;

    using Level = aace::logger::LoggerEngineInterface::Level;

    /**
     * Called when the platform implementation should handle a log event from the SDK logger.
     *
     * @param [in] level The @c aace::logger::Logger::Level log level
     * @param [in] time Time of the logged message
     * @param [in] source The source of the log message
     * @param [in] message The log message
     *
     * @return @c true if log event was handled by the platform.
     */
    virtual bool logEvent( aace::logger::Logger::Level level, std::chrono::system_clock::time_point time, const std::string& source, const std::string& message );
    
    /**
     * Use the Engine's logger to log an event.
     *
     * @param [in] level The @c aace::logger::Logger::Level log level
     * @param [in] tag The log tag
     * @param [in] message The log message
     */
    void log( Level level, const std::string& tag, const std::string& message );

    /**
     * @internal
     * Sets engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<aace::logger::LoggerEngineInterface> loggerEngineInterface );

private:
    std::shared_ptr<aace::logger::LoggerEngineInterface> m_loggerEngineInterface;
};

} // aace::core
} // aace

#endif // AACE_LOGGER_LOGGER_H
