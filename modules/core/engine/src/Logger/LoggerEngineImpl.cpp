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

#include "AACE/Engine/Logger/LoggerEngineImpl.h"
#include "AACE/Engine/Logger/EngineLogger.h"
#include "AACE/Engine/Core/EngineMacros.h"

// String to identify log entries originating from this file.
static const std::string TAG("aace.logger.LoggerEngineImpl");

namespace aace {
namespace engine {
namespace logger {

LoggerEngineImpl::LoggerEngineImpl(std::shared_ptr<aace::logger::Logger> platformLoggerInterface) :
        m_platformLoggerInterface(platformLoggerInterface) {
}

std::shared_ptr<LoggerEngineImpl> LoggerEngineImpl::create(
    std::shared_ptr<aace::logger::Logger> platformLoggerInterface,
    std::shared_ptr<aace::engine::logger::EngineLogger> logger) {
    try {
        auto loggerEngineImpl = std::shared_ptr<LoggerEngineImpl>(new LoggerEngineImpl(platformLoggerInterface));

        ThrowIfNull(loggerEngineImpl, "createLoggerEngineImplFailed");

        // add ourself as an observer to the logger
        logger->addObserver(loggerEngineImpl);

        // set the platform engine interface reference
        platformLoggerInterface->setEngineInterface(loggerEngineImpl);

        return loggerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

bool LoggerEngineImpl::onLogEvent(
    LogEventObserver::Level level,
    std::chrono::system_clock::time_point time,
    const char* source,
    const char* text) {
    return m_platformLoggerInterface != nullptr && m_platformLoggerInterface->logEvent(level, time, source, text);
}

void LoggerEngineImpl::log(aace::logger::Logger::Level level, const std::string& tag, const std::string& message) {
    m_executor.submit([level, tag, message] {
        aace::engine::logger::EngineLogger::getInstance()->log("CLI", level, LX(tag, message));
    });
}

}  // namespace logger
}  // namespace engine
}  // namespace aace
