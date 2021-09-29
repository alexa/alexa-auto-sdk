/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "AACE/Engine/Utils/JSON/JSON.h"
#include "AACE/Engine/Utils/String/StringUtils.h"

namespace aace {
namespace logger {
namespace config {

// json namespace alias
namespace json = aace::engine::utils::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.logger.config.LoggerConfiguationImpl");

std::shared_ptr<aace::core::config::EngineConfiguration> createSinkConfig(
    const std::string& id,
    const std::string& type,
    aace::logger::config::LoggerConfiguration::Level level,
    const json::Value& config = {}) {
    try {
        // clang-format off
        json::Value sink = {
            {"id",id},
            {"type",type},
            {"rules",{
                {{"level",aace::engine::utils::string::from(level)}}
            }},
            {"config",config}
        };
        // clang-format on

        // clang-format off
        json::Value sinkConfig = {
            {"aace.logger",{
                {"sinks",{sink}}
            }}
        };
        // clang-format on

        return aace::core::config::StreamConfiguration::create(json::toStream(sinkConfig));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
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
    try {
        // clang-format off
        json::Value config = {
            {"path",path},
            {"prefix",prefix},
            {"maxSize",maxSize},
            {"maxFiles",maxFiles},
            {"append",append}
        };
        // clang-format on

        return createSinkConfig(id, "aace.logger.sink.file", level, config);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<aace::core::config::EngineConfiguration> LoggerConfiguration::createLoggerRuleConfig(
    const std::string& sink,
    Level level,
    const std::string& sourceFilter,
    const std::string& tagFilter,
    const std::string& messageFilter) {
    try {
        // clang-format off
        json::Value sinkRule = {
            {"sink",sink},
            {"rule",{
                {"level",aace::engine::utils::string::from(level)},
                {"source",sourceFilter},
                {"tag",tagFilter},
                {"message",messageFilter}
            }}
        };
        // clang-format on

        // clang-format off
        json::Value ruleConfig = {
            {"aace.logger",{
                {"rules",{sinkRule}}
            }}
        };
        // clang-format on

        return aace::core::config::StreamConfiguration::create(json::toStream(ruleConfig));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

}  // namespace config
}  // namespace logger
}  // namespace aace
