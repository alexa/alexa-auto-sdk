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

#include "SampleApp/Logger/LoggerHandler.h"

#include "SampleApp/ApplicationContext.h"

// C++ Standard Library
#include <ctime>
#include <iostream>
#include <regex>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace logger {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LoggerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LoggerHandler::LoggerHandler(std::weak_ptr<Activity> activity) : m_activity{std::move(activity)} {
    setupUI();
}

std::weak_ptr<Activity> LoggerHandler::getActivity() {
    return m_activity;
}

// aace::logger::Logger interface

bool LoggerHandler::logEvent(
    Logger::Level level,
    std::chrono::system_clock::time_point time,
    const std::string& source,
    const std::string& message) {
    if (m_applicationContext->isLogEnabled() && (level >= m_applicationContext->getLevel())) {
        if (auto console = m_console.lock()) {
            std::string key{};
            switch (level) {
                case Level::VERBOSE:
                    key = "[V]";
                    break;
                case Level::INFO:
                    key = "[I]";
                    break;
                case Level::METRIC:
                    key = "[M]";
                    break;
                case Level::WARN:
                    key = "[W]";
                    break;
                case Level::ERROR:
                    key = "[E]";
                    break;
                case Level::CRITICAL:
                    key = "[C]";
                    break;
                default:
                    break;
            }
            if (!key.empty()) {
                console->printLine(source, key, message);
                return true;
            }
        }
    }
    return false;
}

// private

void LoggerHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_applicationContext = activity->getApplicationContext();
    m_console = activity->findViewById("id:console");

    // log
    activity->registerObserver(Event::onLoggerLog, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "LoggerHandler", "onLoggerLog:" + value);
        static std::regex r("(.+)/(.+)/(.+)", std::regex::optimize);
        std::smatch sm{};
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 3)) {
            return false;
        }
        // clang-format off
        static const std::map<std::string, Level> LevelEnumerator{
            {"VERBOSE", Level::VERBOSE},
            {"INFO", Level::INFO},
            {"METRIC", Level::METRIC},
            {"WARN", Level::WARN},
            {"ERROR", Level::ERROR},
            {"CRITICAL", Level::CRITICAL}
        };
        // clang-format on
        if (LevelEnumerator.count(sm[1]) == 0) {
            return false;
        }
        log(LevelEnumerator.at(sm[1]), sm[2], sm[3]);
        return true;
    });
}

}  // namespace logger
}  // namespace sampleApp
