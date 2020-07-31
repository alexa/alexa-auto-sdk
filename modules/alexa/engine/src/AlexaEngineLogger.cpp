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

#include <AVSCommon/Utils/Logger/ConsoleLogger.h>
#include <AVSCommon/Utils/Logger/LoggerSinkManager.h>
#include <AVSCommon/Utils/Logger/LoggerUtils.h>
#include <AVSCommon/Utils/Metrics.h>

#include "AACE/Engine/Alexa/AlexaEngineLogger.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlexaEngineLogger");

AlexaEngineLogger::AlexaEngineLogger(alexaClientSDK::avsCommon::utils::logger::Level level) :
        alexaClientSDK::avsCommon::utils::logger::Logger(level),
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG) {
    m_engineLogger = AACE_LOGGER;
    init(alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot()[TAG]);
}

std::shared_ptr<AlexaEngineLogger> AlexaEngineLogger::create(alexaClientSDK::avsCommon::utils::logger::Level level) {
    try {
        std::shared_ptr<AlexaEngineLogger> logger = std::shared_ptr<AlexaEngineLogger>(new AlexaEngineLogger(level));

        // initialize the logger
        alexaClientSDK::avsCommon::utils::logger::LoggerSinkManager::instance().initialize(logger);

        return logger;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

void AlexaEngineLogger::doShutdown() {
    alexaClientSDK::avsCommon::utils::logger::LoggerSinkManager::instance().initialize(
        alexaClientSDK::avsCommon::utils::logger::getConsoleLogger());
}

void AlexaEngineLogger::emit(
    alexaClientSDK::avsCommon::utils::logger::Level level,
    std::chrono::system_clock::time_point time,
    const char* threadMoniker,
    const char* text) {
    aace::logger::Logger::Level aaceLevel = map(level);
    if (aaceLevel == aace::logger::Logger::Level::CRITICAL) {
        if (strstr(text, alexaClientSDK::avsCommon::utils::METRICS_TAG.c_str()) != nullptr) {
            aaceLevel = aace::logger::Logger::Level::METRIC;
        }
    }
    m_engineLogger->log("AVS", TAG, aaceLevel, time, threadMoniker ? threadMoniker : "", text ? text : "");
}

aace::logger::Logger::Level AlexaEngineLogger::map(alexaClientSDK::avsCommon::utils::logger::Level level) {
    switch (level) {
        case alexaClientSDK::avsCommon::utils::logger::Level::INFO:
            return aace::logger::Logger::Level::INFO;
        case alexaClientSDK::avsCommon::utils::logger::Level::WARN:
            return aace::logger::Logger::Level::WARN;
        case alexaClientSDK::avsCommon::utils::logger::Level::ERROR:
            return aace::logger::Logger::Level::ERROR;
        case alexaClientSDK::avsCommon::utils::logger::Level::CRITICAL:
            return aace::logger::Logger::Level::CRITICAL;
        default:
            return aace::logger::Logger::Level::VERBOSE;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
