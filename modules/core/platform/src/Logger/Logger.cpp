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

#include "AACE/Logger/Logger.h"

namespace aace {
namespace logger {

Logger::~Logger() = default;  // key function

bool Logger::logEvent(
    aace::logger::Logger::Level level,
    std::chrono::system_clock::time_point time,
    const std::string& source,
    const std::string& message) {
    return false;
}

void Logger::log(Level level, const std::string& tag, const std::string& message) {
    if (m_loggerEngineInterface != nullptr) {
        m_loggerEngineInterface->log(level, tag, message);
    }
}

void Logger::setEngineInterface(std::shared_ptr<aace::logger::LoggerEngineInterface> loggerEngineInterface) {
    m_loggerEngineInterface = loggerEngineInterface;
}

}  // namespace logger
}  // namespace aace
