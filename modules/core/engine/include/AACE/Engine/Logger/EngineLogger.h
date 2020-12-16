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

#ifndef AACE_ENGINE_LOGGER_ENGINE_LOGGER_H
#define AACE_ENGINE_LOGGER_ENGINE_LOGGER_H

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <memory>
#include <regex>

#include "AACE/Logger/LoggerEngineInterfaces.h"
#include "Sinks/Sink.h"
#include "LogEntry.h"
#include "LogEventObserver.h"

namespace aace {
namespace engine {
namespace logger {

class EngineLogger {
public:
    static std::shared_ptr<EngineLogger> getInstance();

    // EngineLogger::Level alias
    using Level = aace::logger::LoggerEngineInterface::Level;

private:
    EngineLogger();

    /**
     * Emit a log entry.
     * NOTE: This method must be thread-safe.
     * NOTE: Delays in returning from this method may hold up calls to Logger::log().
     *
     * @param [in] level The severity Level of this log line.
     * @param [in] time The time that the event to log occurred.
     * @param [in] threadMoniker Moniker of the thread that generated the event.
     * @param [in] text The text of the entry to log.
     */
    void emit(
        const std::string& source,
        const std::string& tag,
        Level level,
        std::chrono::system_clock::time_point time,
        const char* threadMoniker,
        const char* text);

public:
    virtual ~EngineLogger() = default;

    void addObserver(std::shared_ptr<aace::engine::logger::LogEventObserver> observer);
    void removeObserver(std::shared_ptr<aace::engine::logger::LogEventObserver> observer);
    void log(Level level, const LogEntry& entry);
    void log(const std::string& source, Level level, const LogEntry& entry);
    void log(
        const std::string& source,
        const std::string& tag,
        Level level,
        std::chrono::system_clock::time_point time,
        const std::string& threadMoniker,
        const std::string& text);

private:
    bool addSink(std::shared_ptr<aace::engine::logger::sink::Sink> sink, bool replace = true);
    bool removeSink(const std::string& id);
    std::shared_ptr<aace::engine::logger::sink::Sink> getSink(const std::string& id);

    // allow the LoggerEngineService to configure the EngineLogger
    friend class LoggerEngineService;

private:
    Level m_level;
    std::unordered_set<std::shared_ptr<LogEventObserver>> m_observers;

    // sink map
    std::unordered_map<std::string, std::shared_ptr<aace::engine::logger::sink::Sink>> m_sinkMap;

    // log mutex
    std::mutex m_mutex;

    // singleton
    static std::shared_ptr<EngineLogger> s_instance;
};

}  // namespace logger
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOGGER_ENGINE_LOGGER_H
