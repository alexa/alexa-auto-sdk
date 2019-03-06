/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "LoggerHandler.h"
#include "PlatformSpecificLoggingMacros.h"

namespace aasb {
namespace core {
namespace logger {

std::shared_ptr<LoggerHandler> LoggerHandler::create() {
    auto logHandler = std::shared_ptr<LoggerHandler>(new LoggerHandler());

    return logHandler;
}

bool LoggerHandler::logEvent(
    Level level,
    std::chrono::system_clock::time_point time,
    const std::string& source,
    const std::string& message) {
    switch (level) {
        case Level::VERBOSE:
            AASB_DEBUG("Source: %s, Message: %s", source.c_str(), message.c_str());
        case Level::INFO:
        case Level::METRIC:
            AASB_INFO("Source: %s, Message: %s", source.c_str(), message.c_str());
            break;
        case Level::WARN:
            AASB_WARNING("Source: %s, Message: %s", source.c_str(), message.c_str());
            break;
        case Level::ERROR:
            AASB_ERROR("Source: %s, Message: %s", source.c_str(), message.c_str());
            break;
        case Level::CRITICAL:
            AASB_NOTICE("Source: %s, Message: %s", source.c_str(), message.c_str());
            break;
        default:
            break;
    }

    return true;
}

void LoggerHandler::log(Level level, const std::string& tag, const std::string& message) {
    aace::logger::Logger::log(level, tag, message);
}

}  // namespace logger
}  // namespace core
}  // namespace aasb