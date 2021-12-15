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

#ifndef AACE_ENGINE_MESSAGE_BROKER_MESSAGE_HANDLER_ENGINE_SERVICE_H
#define AACE_ENGINE_MESSAGE_BROKER_MESSAGE_HANDLER_ENGINE_SERVICE_H

#include <vector>
#include <initializer_list>
#include <unordered_map>

#include <AACE/Engine/Core/EngineService.h>

namespace aace {
namespace engine {
namespace messageBroker {

class MessageHandlerEngineService : public aace::engine::core::EngineService {
public:
    enum class Enablement { DEFAULT, DEFAULT_ENABLED, ENABLED, DISABLED };

protected:
    MessageHandlerEngineService(
        const aace::engine::core::ServiceDescription& description,
        const aace::engine::core::Version minRequiredVersion,
        std::initializer_list<std::string> interfaceList,
        std::initializer_list<std::string> defaultEnabledInterfaceList = {});

    bool isInterfaceEnabled(const std::string& name);

    // aace::core::EngineService
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool configure() override;

    // configure the message interface
    virtual bool configureMessageInterface(const std::string& name, bool enabled, std::istream& configuration);

public:
    virtual ~MessageHandlerEngineService() = default;

private:
    std::unordered_map<std::string, Enablement> m_interfaceMap;
    aace::engine::core::Version m_minRequiredVersion;
};

inline std::ostream& operator<<(std::ostream& stream, const MessageHandlerEngineService::Enablement& enablement) {
    switch (enablement) {
        case MessageHandlerEngineService::Enablement::DEFAULT:
            stream << "DEFAULT";
            break;
        case MessageHandlerEngineService::Enablement::DEFAULT_ENABLED:
            stream << "DEFAULT_ENABLED";
            break;
        case MessageHandlerEngineService::Enablement::ENABLED:
            stream << "ENABLED";
            break;
        case MessageHandlerEngineService::Enablement::DISABLED:
            stream << "DISABLED";
            break;
    }
    return stream;
}

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace

#endif
