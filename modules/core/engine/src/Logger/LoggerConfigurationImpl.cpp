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

#include "AACE/Logger/LoggerConfiguration.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

namespace aace {
namespace logger {
namespace config {

// String to identify log entries originating from this file.
static const std::string TAG("aace.logger.config.LoggerConfiguationImpl");

std::shared_ptr<aace::core::config::EngineConfiguration> createSinkConfig(
    const std::string& id,
    const std::string& type,
    aace::logger::config::LoggerConfiguration::Level level,
    rapidjson::Value config = {}) {
    rapidjson::Document document;

    document.SetObject();

    rapidjson::Value aaceLoggerNode(rapidjson::kObjectType);

    // create the sink node
    rapidjson::Value sinkNode(rapidjson::kObjectType);

    sinkNode.AddMember("id", rapidjson::Value().SetString(id.c_str(), id.length()), document.GetAllocator());
    sinkNode.AddMember("type", rapidjson::Value().SetString(type.c_str(), type.length()), document.GetAllocator());

    if (config.IsObject()) {
        sinkNode.AddMember("config", config, document.GetAllocator());
    }

    // create the rule node
    rapidjson::Value ruleNode(rapidjson::kObjectType);

    // convert the level to a string
    std::string levelStr;

    switch (level) {
        case aace::logger::config::LoggerConfiguration::Level::METRIC:
            levelStr = "METRIC";
            break;
        case aace::logger::config::LoggerConfiguration::Level::CRITICAL:
            levelStr = "CRITICAL";
            break;
        case aace::logger::config::LoggerConfiguration::Level::ERROR:
            levelStr = "ERROR";
            break;
        case aace::logger::config::LoggerConfiguration::Level::WARN:
            levelStr = "WARN";
            break;
        case aace::logger::config::LoggerConfiguration::Level::INFO:
            levelStr = "INFO";
            break;
        case aace::logger::config::LoggerConfiguration::Level::VERBOSE:
            levelStr = "VERBOSE";
            break;
    }

    ruleNode.AddMember(
        "level", rapidjson::Value().SetString(levelStr.c_str(), levelStr.length()), document.GetAllocator());

    // create the rules list node and add the rule to the list
    rapidjson::Value rulesListNode(rapidjson::kArrayType);

    rulesListNode.PushBack(ruleNode, document.GetAllocator());

    // add the rules list to the sink node
    sinkNode.AddMember("rules", rulesListNode, document.GetAllocator());

    // create the sinks list node and add the sink to the list
    rapidjson::Value sinkListNode(rapidjson::kArrayType);

    sinkListNode.PushBack(sinkNode, document.GetAllocator());

    // add the sinks list to the aace logger node
    aaceLoggerNode.AddMember("sinks", sinkListNode, document.GetAllocator());

    // add the aace logger node to the root object
    document.AddMember("aace.logger", aaceLoggerNode, document.GetAllocator());

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    return aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(buffer.GetString()));
}

std::shared_ptr<aace::core::config::EngineConfiguration> LoggerConfiguration::createConsoleSinkConfig(
    const std::string& id,
    Level level) {
    return createSinkConfig(id, "aace.logger.sink.console", level);
}

std::shared_ptr<aace::core::config::EngineConfiguration> LoggerConfiguration::createSyslogSinkConfig(
    const std::string& id,
    Level level) {
    return createSinkConfig(id, "aace.logger.sink.syslog", level);
}

std::shared_ptr<aace::core::config::EngineConfiguration> LoggerConfiguration::createFileSinkConfig(
    const std::string& id,
    Level level,
    const std::string& path,
    const std::string& prefix,
    uint32_t maxSize,
    uint32_t maxFiles,
    bool append) {
    rapidjson::Document document;

    document.SetObject();

    auto configNode = document.GetObject();

    configNode.AddMember("path", rapidjson::Value().SetString(path.c_str(), path.length()), document.GetAllocator());
    configNode.AddMember(
        "prefix", rapidjson::Value().SetString(prefix.c_str(), prefix.length()), document.GetAllocator());
    configNode.AddMember("maxSize", maxSize, document.GetAllocator());
    configNode.AddMember("maxFiles", maxFiles, document.GetAllocator());
    configNode.AddMember("append", append, document.GetAllocator());

    return createSinkConfig(id, "aace.logger.sink.file", level, configNode);
}

std::shared_ptr<aace::core::config::EngineConfiguration> LoggerConfiguration::createLoggerRuleConfig(
    const std::string& sink,
    Level level,
    const std::string& sourceFilter,
    const std::string& tagFilter,
    const std::string& messageFilter) {
    rapidjson::Document document;

    document.SetObject();

    rapidjson::Value aaceLoggerNode(rapidjson::kObjectType);

    // create the default sink rule node
    rapidjson::Value defaultSinkRuleNode(rapidjson::kObjectType);

    defaultSinkRuleNode.AddMember(
        "sink", rapidjson::Value().SetString(sink.c_str(), sink.length()), document.GetAllocator());

    // create the rule node
    rapidjson::Value ruleNode(rapidjson::kObjectType);

    // convert the level to a string
    std::string levelStr;

    switch (level) {
        case aace::logger::config::LoggerConfiguration::Level::METRIC:
            levelStr = "METRIC";
            break;
        case aace::logger::config::LoggerConfiguration::Level::CRITICAL:
            levelStr = "CRITICAL";
            break;
        case aace::logger::config::LoggerConfiguration::Level::ERROR:
            levelStr = "ERROR";
            break;
        case aace::logger::config::LoggerConfiguration::Level::WARN:
            levelStr = "WARN";
            break;
        case aace::logger::config::LoggerConfiguration::Level::INFO:
            levelStr = "INFO";
            break;
        case aace::logger::config::LoggerConfiguration::Level::VERBOSE:
            levelStr = "VERBOSE";
            break;
    }

    ruleNode.AddMember(
        "level", rapidjson::Value().SetString(levelStr.c_str(), levelStr.length()), document.GetAllocator());

    if (sourceFilter.empty() == false) {
        ruleNode.AddMember(
            "source",
            rapidjson::Value().SetString(sourceFilter.c_str(), sourceFilter.length()),
            document.GetAllocator());
    }

    if (tagFilter.empty() == false) {
        ruleNode.AddMember(
            "tag", rapidjson::Value().SetString(tagFilter.c_str(), tagFilter.length()), document.GetAllocator());
    }

    if (messageFilter.empty() == false) {
        ruleNode.AddMember(
            "message",
            rapidjson::Value().SetString(messageFilter.c_str(), messageFilter.length()),
            document.GetAllocator());
    }

    defaultSinkRuleNode.AddMember("rule", ruleNode, document.GetAllocator());

    // create the sinks list node and add the sink to the list
    rapidjson::Value rulesListNode(rapidjson::kArrayType);

    rulesListNode.PushBack(defaultSinkRuleNode, document.GetAllocator());

    // add the sinks list to the aace logger node
    aaceLoggerNode.AddMember("rules", rulesListNode, document.GetAllocator());

    // add the aace logger node to the root object
    document.AddMember("aace.logger", aaceLoggerNode, document.GetAllocator());

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    return aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(buffer.GetString()));
}

}  // namespace config
}  // namespace logger
}  // namespace aace
