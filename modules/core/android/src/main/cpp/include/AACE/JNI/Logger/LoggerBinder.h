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

#ifndef AACE_JNI_LOGGER_LOGGER_BINDER_H
#define AACE_JNI_LOGGER_LOGGER_BINDER_H

#include <AACE/Logger/Logger.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

#include <chrono>

namespace aace {
namespace jni {
namespace logger {

//
// LoggerHandler
//

class LoggerHandler : public aace::logger::Logger {
public:
    LoggerHandler(jobject obj);

    // aace::logger::Logger
    bool logEvent(
        Level level,
        std::chrono::system_clock::time_point time,
        const std::string& source,
        const std::string& message) override;

private:
    JObject m_obj;
};

//
// LoggerBinder
//

class LoggerBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    LoggerBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_loggerHandler;
    }

    std::shared_ptr<LoggerHandler> getLoggerHandler() {
        return m_loggerHandler;
    }

private:
    std::shared_ptr<LoggerHandler> m_loggerHandler;
};

//
// JLogLevel
//

class JLogLevelConfig : public EnumConfiguration<LoggerHandler::Level> {
public:
    using T = LoggerHandler::Level;

    const char* getClassName() override {
        return "com/amazon/aace/logger/Logger$Level";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::VERBOSE, "VERBOSE"},
                {T::INFO, "INFO"},
                {T::METRIC, "METRIC"},
                {T::WARN, "WARN"},
                {T::ERROR, "ERROR"},
                {T::CRITICAL, "CRITICAL"}};
    }
};

using JLogLevel = JEnum<LoggerHandler::Level, JLogLevelConfig>;

}  // namespace logger
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_LOGGER_LOGGER_BINDER_H
