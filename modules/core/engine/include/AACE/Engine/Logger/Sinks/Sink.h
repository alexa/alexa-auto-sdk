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

#ifndef AACE_ENGINE_LOGGER_SINK_SINK_H
#define AACE_ENGINE_LOGGER_SINK_SINK_H

#include <memory>
#include <streambuf>
#include <vector>
#include <chrono>
#include <mutex>
#include <fstream>
#include <regex>

#include "AACE/Logger/LoggerEngineInterfaces.h"

namespace aace {
namespace engine {
namespace logger {
namespace sink {

class Rule;

class Sink {
public:
    using Level = aace::logger::LoggerEngineInterface::Level;

protected:
    Sink(const std::string& id);

public:
    virtual ~Sink() = default;

    virtual void log(
        Level level,
        std::chrono::system_clock::time_point time,
        const char* threadMoniker,
        const char* text) = 0;
    virtual void flush();

    std::string getId();

    bool addRule(std::shared_ptr<Rule> rule, bool replace = true);
    bool addRule(
        Level level,
        const std::string& source,
        const std::string& tag,
        const std::string& message,
        bool replace = true);
    bool addRule(
        const std::string& level,
        const std::string& source,
        const std::string& tag,
        const std::string& message,
        bool replace = true);

    void emit(
        const std::string& source,
        const std::string& tag,
        Level level,
        std::chrono::system_clock::time_point time,
        const char* threadMoniker,
        const char* text);

private:
    std::string m_id;
    std::vector<std::shared_ptr<Rule>> m_rules;
};

//
// Rule
//
class Rule {
public:
    using Level = aace::logger::LoggerEngineInterface::Level;

    // define constant empty rule pattern
    static const std::string EMPTY;

private:
    Rule(Level level, const std::string& source, const std::string& tag, const std::string& message);

public:
    static std::shared_ptr<Rule> create(
        Level level,
        const std::string& source,
        const std::string& tag,
        const std::string& message);
    static std::shared_ptr<Rule> create(
        const std::string& level,
        const std::string& source,
        const std::string& tag,
        const std::string& message);

    bool equals(const Rule& rule);
    bool match(Level level, const std::string& source, const std::string& tag, const char* text);

private:
    Sink::Level m_level;
    std::string m_source;
    std::regex m_sourceRegex;
    std::string m_tag;
    std::regex m_tagRegex;
    std::string m_message;
    std::regex m_messageRegex;
};

}  // namespace sink
}  // namespace logger
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOGGER_SINK_SINK_H
