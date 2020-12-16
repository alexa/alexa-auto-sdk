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

#include <iostream>
#include <sstream>

#include "AACE/Engine/Logger/EngineLogger.h"
#include "AACE/Engine/Logger/LogFormatter.h"
#include "AACE/Engine/Logger/ThreadMoniker.h"
#ifdef AAC_DEFAULT_LOGGER_SINK_CONSOLE
#include "AACE/Engine/Logger/Sinks/ConsoleSink.h"
#endif
#ifdef AAC_DEFAULT_LOGGER_SINK_SYSLOG
#include "AACE/Engine/Logger/Sinks/SyslogSink.h"
#endif
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace logger {

std::shared_ptr<EngineLogger> EngineLogger::getInstance() {
    static std::shared_ptr<EngineLogger> s_instance(new EngineLogger());
    return s_instance;
}

EngineLogger::EngineLogger() {
#ifdef AAC_DEFAULT_LOGGER_ENABLED
#ifdef AAC_DEFAULT_LOGGER_SINK
#if defined AAC_DEFAULT_LOGGER_SINK_CONSOLE
    auto sink = aace::engine::logger::sink::ConsoleSink::create("default");
#elif defined AAC_DEFAULT_LOGGER_SINK_SYSLOG
    auto sink = aace::engine::logger::sink::SyslogSink::create("default");
#else
#error "Unknown logger sink"
#endif

#ifdef AAC_DEFAULT_LOGGER_LEVEL
#if defined AAC_DEFAULT_LOGGER_LEVEL_VERBOSE
    auto rule = aace::engine::logger::sink::Rule::create(
        Level::VERBOSE,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY);
#elif defined AAC_DEFAULT_LOGGER_LEVEL_INFO
    auto rule = aace::engine::logger::sink::Rule::create(
        Level::INFO,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY);
#elif defined AAC_DEFAULT_LOGGER_LEVEL_METRIC
    auto rule = aace::engine::logger::sink::Rule::create(
        Level::METRIC,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY);
#elif defined AAC_DEFAULT_LOGGER_LEVEL_WARN
    auto rule = aace::engine::logger::sink::Rule::create(
        Level::WARN,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY);
#elif defined AAC_DEFAULT_LOGGER_LEVEL_ERROR
    auto rule = aace::engine::logger::sink::Rule::create(
        Level::ERROR,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY);
#elif defined AAC_DEFAULT_LOGGER_LEVEL_CRITICAL
    auto rule = aace::engine::logger::sink::Rule::create(
        Level::CRITICAL,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY,
        aace::engine::logger::sink::Rule::EMPTY);
#else
#error "Unknown logger level"
#endif

    // add the default rule to the sink
    sink->addRule(rule, false);

#endif  // AAC_DEFAULT_LOGGER_LEVEL

    // add the console sink to the logger
    addSink(sink, false);

#endif  // AAC_DEFAULT_LOGGER_SINK
#endif  // AAC_DEFAULT_LOGGER_ENABLED
}

void EngineLogger::addObserver(std::shared_ptr<aace::engine::logger::LogEventObserver> observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.insert(observer);
}

void EngineLogger::removeObserver(std::shared_ptr<aace::engine::logger::LogEventObserver> observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.erase(observer);
}

void EngineLogger::log(Level level, const LogEntry& entry) {
    emit(
        "AAC",
        entry.tag(),
        level,
        std::chrono::system_clock::now(),
        ThreadMoniker::getThisThreadMoniker(),
        entry.c_str());
}

void EngineLogger::log(const std::string& source, Level level, const LogEntry& entry) {
    emit(
        source,
        entry.tag(),
        level,
        std::chrono::system_clock::now(),
        ThreadMoniker::getThisThreadMoniker(),
        entry.c_str());
}

void EngineLogger::log(
    const std::string& source,
    const std::string& tag,
    Level level,
    std::chrono::system_clock::time_point time,
    const std::string& threadMoniker,
    const std::string& text) {
    emit(source, tag, level, time, threadMoniker.c_str(), text.c_str());
}

void EngineLogger::emit(
    const std::string& source,
    const std::string& tag,
    Level level,
    std::chrono::system_clock::time_point time,
    const char* threadMoniker,
    const char* text) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // iterate through each register sink and emit the log entry
    for (auto it = m_sinkMap.begin(); it != m_sinkMap.end(); it++) {
        it->second->emit(source, tag, level, time, threadMoniker, text);
    }

    // iterate through all of the log event observers and log the message
    // to each observer in the list
    for (auto next : m_observers) {
        next->onLogEvent(level, time, source.c_str(), text);
    }
}

bool EngineLogger::addSink(std::shared_ptr<aace::engine::logger::sink::Sink> sink, bool replace) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (replace || m_sinkMap.find(sink->getId()) == m_sinkMap.end()) {
        m_sinkMap[sink->getId()] = sink;
        return true;
    } else {
        return false;
    }
}

std::shared_ptr<aace::engine::logger::sink::Sink> EngineLogger::getSink(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sinkMap.find(id) != m_sinkMap.end() ? m_sinkMap[id] : nullptr;
}

bool EngineLogger::removeSink(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sinkMap.find(id);

    if (it != m_sinkMap.end()) {
        m_sinkMap.erase(it);
    }

    return true;
}

}  // namespace logger
}  // namespace engine
}  // namespace aace
