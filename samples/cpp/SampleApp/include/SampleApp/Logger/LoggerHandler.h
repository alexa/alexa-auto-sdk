/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Logger/Logger.h>

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

class LoggerHandler : public aace::logger::Logger /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};

protected:
    LoggerHandler(std::weak_ptr<Activity> activity);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<LoggerHandler> {
        return std::shared_ptr<LoggerHandler>(new LoggerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;

    // aace::logger::Logger interface

    auto logEvent(
        Logger::Level level,
        std::chrono::system_clock::time_point time,
        const std::string& source,
        const std::string& message) -> bool override;

private:
    std::shared_ptr<ApplicationContext> m_applicationContext{};
    std::weak_ptr<View> m_console{};

    auto setupUI() -> void;
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
