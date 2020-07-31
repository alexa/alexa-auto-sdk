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

#ifndef AACE_ENGINE_ALEXA_ALEXA_ENGINE_LOGGER_H
#define AACE_ENGINE_ALEXA_ALEXA_ENGINE_LOGGER_H

#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AACE/Engine/Logger/EngineLogger.h>

#include "AACE/Logger/Logger.h"

namespace aace {
namespace engine {
namespace alexa {

class AlexaEngineLogger
        : public alexaClientSDK::avsCommon::utils::logger::Logger
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown {
private:
    AlexaEngineLogger(alexaClientSDK::avsCommon::utils::logger::Level level);

protected:
    virtual void doShutdown() override;

public:
    static std::shared_ptr<AlexaEngineLogger> create(alexaClientSDK::avsCommon::utils::logger::Level level);

    virtual void emit(
        alexaClientSDK::avsCommon::utils::logger::Level level,
        std::chrono::system_clock::time_point time,
        const char* threadMoniker,
        const char* text) override;

private:
    aace::logger::Logger::Level map(alexaClientSDK::avsCommon::utils::logger::Level level);
    std::shared_ptr<aace::engine::logger::EngineLogger> m_engineLogger;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_ALEXA_ENGINE_LOGGER_H
