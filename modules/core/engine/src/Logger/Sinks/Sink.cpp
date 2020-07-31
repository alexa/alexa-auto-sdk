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

#include "AACE/Engine/Logger/Sinks/Sink.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace logger {
namespace sink {

// String to identify log entries originating from this file.
static const std::string TAG("aace.logger.sink.Sink");

Sink::Sink(const std::string& id) : m_id(id) {
}

bool Sink::addRule(std::shared_ptr<Rule> rule, bool replace) {
    for (auto it = m_rules.begin(); it != m_rules.end(); it++) {
        if ((*it)->equals(*rule)) {
            ReturnIfNot(replace, false);
            m_rules.erase(it);
            break;
        }
    }

    m_rules.push_back(rule);

    return true;
}

bool Sink::addRule(
    Level level,
    const std::string& source,
    const std::string& tag,
    const std::string& message,
    bool replace) {
    return addRule(Rule::create(level, source, tag, message), replace);
}

bool Sink::addRule(
    const std::string& level,
    const std::string& source,
    const std::string& tag,
    const std::string& message,
    bool replace) {
    return addRule(Rule::create(level, source, tag, message), replace);
}

void Sink::emit(
    const std::string& source,
    const std::string& tag,
    Level level,
    std::chrono::system_clock::time_point time,
    const char* threadMoniker,
    const char* text) {
    for (auto next : m_rules) {
        if (next->match(level, source, tag, text)) {
            log(level, time, source.c_str(), text);
            break;
        }
    }
}

void Sink::flush() {
}

std::string Sink::getId() {
    return m_id;
}

//
// Rule
//
const std::string Rule::EMPTY;

Rule::Rule(Level level, const std::string& source, const std::string& tag, const std::string& message) :
        m_level(level),
        m_source(source),
        m_sourceRegex(source),
        m_tag(tag),
        m_tagRegex(tag),
        m_message(message),
        m_messageRegex(message) {
}

std::shared_ptr<Rule> Rule::create(
    Level level,
    const std::string& source,
    const std::string& tag,
    const std::string& message) {
    return std::shared_ptr<Rule>(new Rule(level, source, tag, message));
}

std::shared_ptr<Rule> Rule::create(
    const std::string& level,
    const std::string& source,
    const std::string& tag,
    const std::string& message) {
    std::string lvlUp = level;
    std::transform(lvlUp.begin(), lvlUp.end(), lvlUp.begin(), [](unsigned char c) -> unsigned char {
        return static_cast<unsigned char>(std::toupper(c));
    });
    Level lv;

    if (lvlUp.compare("CRITICAL") == 0) {
        lv = Level::CRITICAL;
    } else if (lvlUp.compare("ERROR") == 0) {
        lv = Level::ERROR;
    } else if (lvlUp.compare("WARN") == 0) {
        lv = Level::WARN;
    } else if (lvlUp.compare("INFO") == 0) {
        lv = Level::INFO;
    } else if (lvlUp.compare("METRIC") == 0) {
        lv = Level::METRIC;
    } else {
        lv = Level::VERBOSE;
    }

    return std::shared_ptr<Rule>(new Rule(lv, source, tag, message));
}

bool Rule::equals(const Rule& rule) {
    return m_source.compare(rule.m_source) == 0 && m_tag.compare(rule.m_tag) == 0 &&
           m_message.compare(rule.m_message) == 0;
}

bool Rule::match(Level level, const std::string& source, const std::string& tag, const char* text) {
    return level >= m_level && (m_source.empty() || std::regex_match(source, m_sourceRegex)) &&
           (m_tag.empty() || std::regex_match(tag, m_tagRegex)) &&
           (m_message.empty() || std::regex_match(text, m_messageRegex));
}

}  // namespace sink
}  // namespace logger
}  // namespace engine
}  // namespace aace
