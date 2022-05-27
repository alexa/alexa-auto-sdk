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

#include "SampleApp/Logger/LoggerHandler.h"

#include "SampleApp/ApplicationContext.h"

// C++ Standard Library
#include <ctime>
#include <iostream>
#include <regex>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace logger {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LoggerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LoggerHandler::LoggerHandler(std::weak_ptr<Activity> activity) {
    auto activity_lock = activity.lock();
    if (!activity_lock) {
        return;
    }
    m_console = activity_lock->findViewById("id:console");
}

void LoggerHandler::log(Level level, const std::string& tag, const std::string& message) {
    if (auto console = m_console.lock()) {
        std::string key{};
        switch (level) {
            case Level::VERBOSE:
                key = "V";
                break;
            case Level::INFO:
                key = "I";
                break;
            case Level::METRIC:
                key = "M";
                break;
            case Level::WARN:
                key = "W";
                break;
            case Level::ERROR:
                key = "E";
                break;
            case Level::CRITICAL:
                key = "C";
                break;
            default:
                break;
        }
        if (!key.empty()) {
            console->printLine(formatTime(), "[CLI]", key, tag + ":" + message);
        }
    }
}

std::string LoggerHandler::formatTime() {
    std::stringstream stringToEmit;

    // Format string using strftime() to produce date and time in the format "YYYY-MM-DD HH:MM:SS".
    std::chrono::system_clock::time_point time = std::chrono::system_clock::now();
    char dateTimeString[20];
    auto timeAsTime_t = std::chrono::system_clock::to_time_t(time);
    auto timeAsTmPtr = std::gmtime(&timeAsTime_t);
    strftime(dateTimeString, sizeof(dateTimeString), "%Y-%m-%d %H:%M:%S", timeAsTmPtr);

    // Format string using sprintf() to produce milliseconds in the format "nnn".
    char millisString[4];
    auto timeMillisPart =
        static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count() % 1000);
    std::snprintf(millisString, sizeof(millisString), "%03d", timeMillisPart);

    stringToEmit << dateTimeString << '.' << millisString;

    return stringToEmit.str();
}

}  // namespace logger
}  // namespace sampleApp
