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

#ifndef AACE_ENGINE_LOGGER_LOGGER_ENGINE_IMPL_H
#define AACE_ENGINE_LOGGER_LOGGER_ENGINE_IMPL_H

#include <chrono>
#include <memory>
#include <mutex>

#include <AACE/Engine/Utils/Threading/Executor.h>
#include <AACE/Logger/Logger.h>

#include "EngineLogger.h"
#include "LogEntry.h"
#include "LogEventObserver.h"

namespace aace {
namespace engine {
namespace logger {

class LoggerEngineImpl
        : public aace::logger::LoggerEngineInterface
        , public LogEventObserver {
public:
    virtual ~LoggerEngineImpl() = default;

    static std::shared_ptr<LoggerEngineImpl> create(
        std::shared_ptr<aace::logger::Logger> platformLoggerInterface,
        std::shared_ptr<aace::engine::logger::EngineLogger> logger);

private:
    LoggerEngineImpl(std::shared_ptr<aace::logger::Logger> platformLoggerInterface);

public:
    // LogEventObserver
    virtual bool onLogEvent(
        LogEventObserver::Level level,
        std::chrono::system_clock::time_point time,
        const char* source,
        const char* text) override;

    // LoggerEngineInterface
    virtual void log(aace::logger::Logger::Level level, const std::string& tag, const std::string& message) override;

private:
    std::shared_ptr<aace::logger::Logger> m_platformLoggerInterface;

    // executor
    aace::engine::utils::threading::Executor m_executor;
};

}  // namespace logger
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOGGER_LOGGER_ENGINE_IMPL_H
