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

#include <typeinfo>
#include <algorithm>
#include <iostream>

#include "AACE/Engine/Logger/LoggerEngineService.h"
#include "AACE/Engine/Logger/Sinks/Sink.h"
#include "AACE/Engine/Logger/Sinks/ConsoleSink.h"
#include "AACE/Engine/Logger/Sinks/FileSink.h"
#include "AACE/Engine/Logger/Sinks/SyslogSink.h"
#include "AACE/Engine/Utils/JSON/JSON.h"
#include "AACE/Engine/Utils/String/StringUtils.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace logger {

// json alias
namespace json = aace::engine::utils::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.logger.LoggerEngineService");

// register the service
REGISTER_SERVICE(LoggerEngineService);

LoggerEngineService::LoggerEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

// LoggerServiceInterface
bool LoggerEngineService::addSink(std::shared_ptr<aace::engine::logger::sink::Sink> sink) {
    return EngineLogger::getInstance()->addSink(sink);
}

bool LoggerEngineService::removeSink(const std::string& id) {
    return EngineLogger::getInstance()->removeSink(id);
}

bool LoggerEngineService::initialize() {
    try {
        ThrowIfNot(
            registerServiceInterface<LoggerServiceInterface>(shared_from_this()),
            "registerLoggerServiceInterfaceFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

bool LoggerEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        auto root = json::toJson(configuration);
        ThrowIfNull(root, "parseConfigurationFailed");

        auto sinkConfigList = json::get(root, "/sinks", json::Type::array);
        if (sinkConfigList != nullptr) {
            for (std::size_t j = 0; j < sinkConfigList.size(); j++) {
                auto sink = createSink(sinkConfigList[j]);
                ThrowIfNull(sink, "createSinkFailed");
                // add the sink to the engine logger
                if (!EngineLogger::getInstance()->addSink(sink)) {
                    AACE_ERROR(LX(TAG).m("addSinkFailed"));
                }
            }
        }

        auto rulesConfigList = json::get(root, "/rules", json::Type::array);
        if (rulesConfigList != nullptr) {
            for (std::size_t j = 0; j < rulesConfigList.size(); j++) {
                auto next = rulesConfigList[j];
                auto sinkId = json::get(next, "/sink", json::Type::string);
                if (sinkId != nullptr) {
                    auto sink = EngineLogger::getInstance()->getSink(sinkId);
                    ThrowIfNull(sink, "invalidSink");
                    auto ruleConfig = json::get(next, "/rule", json::Type::object);
                    if (ruleConfig != nullptr) {
                        auto rule = createRule(ruleConfig);
                        ThrowIfNull(rule, "createRuleFailed");
                        sink->addRule(rule);
                    }
                }
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<aace::engine::logger::sink::Sink> LoggerEngineService::createSink(
    const aace::engine::utils::json::Value& config) {
    try {
        auto id = json::get(config, "/id", json::Type::string);
        ThrowIfNull(id, "invalidOrMissingId");

        auto type = json::get(config, "/type", json::Type::string);
        ThrowIfNull(type, "invalidOrMissingType");

        // create the new sink
        std::shared_ptr<aace::engine::logger::sink::Sink> sink = nullptr;

        if (aace::engine::utils::string::equal(type, "aace.logger.sink.console", false)) {
            sink = aace::engine::logger::sink::ConsoleSink::create(id);
        } else if (aace::engine::utils::string::equal(type, "aace.logger.sink.syslog", false)) {
            sink = aace::engine::logger::sink::SyslogSink::create(id);
        } else if (aace::engine::utils::string::equal(type, "aace.logger.sink.file", false)) {
            auto path = json::get(config, "/config/path", json::Type::string);
            ThrowIfNull(path, "invalidOrMissingConfigPath");

            std::string prefix = json::get(config, "/config/prefix", "aace");
            uint32_t maxSize = json::get(config, "/config/maxSize", (uint64_t)1048576);
            uint32_t maxFiles = json::get(config, "/config/maxFiles", (uint64_t)3);
            bool append = json::get(config, "/config/append", true);

            sink = aace::engine::logger::sink::FileSink::create(id, path, prefix, maxSize, maxFiles, append);
        } else {
            Throw("invalidSinkType");
        }

        // fail if the sink could not be created
        ThrowIfNull(sink, "createSinkFailed");

        auto rulesConfigList = json::get(config, "/rules", json::Type::array);
        if (rulesConfigList != nullptr) {
            for (std::size_t j = 0; j < rulesConfigList.size(); j++) {
                auto rule = createRule(rulesConfigList[j]);
                ThrowIfNull(rule, "createRuleFailed");
                sink->addRule(rule);
            }
        }

        return sink;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<aace::engine::logger::sink::Rule> LoggerEngineService::createRule(
    const aace::engine::utils::json::Value& config) {
    try {
        std::string level = json::get(config, "/level", aace::engine::logger::sink::Rule::EMPTY);
        std::string source = json::get(config, "/source", aace::engine::logger::sink::Rule::EMPTY);
        std::string tag = json::get(config, "/tag", aace::engine::logger::sink::Rule::EMPTY);
        std::string message = json::get(config, "/message", aace::engine::logger::sink::Rule::EMPTY);

        return aace::engine::logger::sink::Rule::create(level, source, tag, message);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createRule").d("reason", ex.what()));
        return nullptr;
    }
}

bool LoggerEngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::logger::Logger>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool LoggerEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::logger::Logger> logger) {
    try {
        ThrowIfNotNull(m_loggerEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the logger engine implementation
        m_logger = logger;
        m_loggerEngineImpl = aace::engine::logger::LoggerEngineImpl::create(logger, EngineLogger::getInstance());
        ThrowIfNull(m_loggerEngineImpl, "createLoggerEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<Logger>").d("reason", ex.what()));
        return false;
    }
}

bool LoggerEngineService::shutdown() {
    if (m_logger != nullptr) {
        m_logger->setEngineInterface(nullptr);
        m_logger.reset();
        m_logger = nullptr;
    }
    if (m_loggerEngineImpl != nullptr) {
        EngineLogger::getInstance()->removeObserver(m_loggerEngineImpl);
        m_loggerEngineImpl.reset();
        m_loggerEngineImpl = nullptr;
    }
    return true;
}

}  // namespace logger
}  // namespace engine
}  // namespace aace
