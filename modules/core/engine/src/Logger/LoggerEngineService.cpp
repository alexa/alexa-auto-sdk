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

#include <typeinfo>
#include <algorithm>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include "AACE/Engine/Logger/LoggerEngineService.h"
#include "AACE/Engine/Logger/Sinks/Sink.h"
#include "AACE/Engine/Logger/Sinks/ConsoleSink.h"
#include "AACE/Engine/Logger/Sinks/FileSink.h"
#include "AACE/Engine/Logger/Sinks/SyslogSink.h"
#include "AACE/Engine/Utils/JSON/JSON.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace logger {

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
        auto document = aace::engine::utils::json::parse(configuration);
        ThrowIfNull(document, "parseConfigurationStreamFailed");

        auto loggerConfigRoot = document->GetObject();

        if (loggerConfigRoot.HasMember("sinks") && loggerConfigRoot["sinks"].IsArray()) {
            auto sinks = loggerConfigRoot["sinks"].GetArray();

            for (unsigned int j = 0; j < sinks.Size(); j++) {
                auto sink = createSink(sinks[j]);

                // add the sink to the engine logger
                if (sink != nullptr) {
                    EngineLogger::getInstance()->addSink(sink);
                }
            }
        }

        if (loggerConfigRoot.HasMember("rules") && loggerConfigRoot["rules"].IsArray()) {
            auto rules = loggerConfigRoot["rules"].GetArray();

            for (unsigned int j = 0; j < rules.Size(); j++) {
                auto obj = rules[j].GetObject();

                if (obj.HasMember("sink") && obj["sink"].IsString() && obj.HasMember("rule") &&
                    obj["rule"].IsObject()) {
                    auto sink = EngineLogger::getInstance()->getSink(obj["sink"].GetString());

                    if (sink != nullptr) {
                        auto rule = createRule(obj["rule"].GetObject());

                        if (rule != nullptr) {
                            sink->addRule(rule);
                        }
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

std::shared_ptr<aace::engine::logger::sink::Sink> LoggerEngineService::createSink(const rapidjson::Value& config) {
    try {
        auto obj = config.GetObject();

        ThrowIfNot(obj.HasMember("id") && obj["id"].IsString(), "invalidOrMissingConfigData");
        ThrowIfNot(obj.HasMember("type") && obj["type"].IsString(), "invalidOrMissingConfigData");

        std::string id = obj["id"].GetString();
        std::string type = obj["type"].GetString();

        // convert the type to lower case
        std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) -> unsigned char {
            return static_cast<unsigned char>(std::tolower(c));
        });

        // create the new sink
        std::shared_ptr<aace::engine::logger::sink::Sink> sink = nullptr;

        if (type == "aace.logger.sink.console") {
            sink = aace::engine::logger::sink::ConsoleSink::create(id);
        } else if (type == "aace.logger.sink.syslog") {
            sink = aace::engine::logger::sink::SyslogSink::create(id);
        } else if (type == "aace.logger.sink.file") {
            ThrowIfNot(obj.HasMember("config") && obj["config"].IsObject(), "invalidOrMissingConfigData");

            auto config = obj["config"].GetObject();

            ThrowIfNot(config.HasMember("path") && config["path"].IsString(), "invalidOrMissingConfigData");

            std::string path = config["path"].GetString();
            std::string prefix =
                config.HasMember("prefix") && config["prefix"].IsString() ? config["prefix"].GetString() : "aace";
            uint32_t maxSize =
                config.HasMember("maxSize") && config["maxSize"].IsUint() ? config["maxSize"].GetUint() : 1048576;
            uint32_t maxFiles =
                config.HasMember("maxFiles") && config["maxFiles"].IsUint() ? config["maxFiles"].GetUint() : 3;
            bool append = config.HasMember("append") && config["append"].IsBool() ? config["append"].GetBool() : true;

            sink = aace::engine::logger::sink::FileSink::create(id, path, prefix, maxSize, maxFiles, append);
        } else {
            Throw("invalideSinkType");
        }

        // fail if the sink could not be created
        ThrowIfNull(sink, "couldNotCreateSink");

        if (obj.HasMember("rules") && obj["rules"].IsArray()) {
            auto rules = obj["rules"].GetArray();

            for (unsigned int j = 0; j < rules.Size(); j++) {
                auto rule = createRule(rules[j]);

                if (rule != nullptr) {
                    sink->addRule(rule);
                }
            }
        }

        return sink;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createSink").d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<aace::engine::logger::sink::Rule> LoggerEngineService::createRule(const rapidjson::Value& config) {
    try {
        auto obj = config.GetObject();

        std::string level = obj.HasMember("level") && obj["level"].IsString() ? obj["level"].GetString()
                                                                              : aace::engine::logger::sink::Rule::EMPTY;
        std::string source = obj.HasMember("source") && obj["source"].IsString()
                                 ? obj["source"].GetString()
                                 : aace::engine::logger::sink::Rule::EMPTY;
        std::string tag = obj.HasMember("tag") && obj["tag"].IsString() ? obj["tag"].GetString()
                                                                        : aace::engine::logger::sink::Rule::EMPTY;
        std::string message = obj.HasMember("message") && obj["message"].IsString()
                                  ? obj["message"].GetString()
                                  : aace::engine::logger::sink::Rule::EMPTY;

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
