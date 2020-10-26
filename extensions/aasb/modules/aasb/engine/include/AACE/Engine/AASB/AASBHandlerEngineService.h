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

#ifndef AACE_ENGINE_AASB_AASB_HANDLER_ENGINE_SERVICE_H
#define AACE_ENGINE_AASB_AASB_HANDLER_ENGINE_SERVICE_H

#include <vector>
#include <initializer_list>
#include <unordered_map>

#include <AACE/Engine/Core/EngineService.h>

namespace aace {
namespace engine {
namespace aasb {

class AASBHandlerEngineService : public aace::engine::core::EngineService {
public:
    enum class Enablement { DEFAULT, ENABLED, DISABLED };

protected:
    AASBHandlerEngineService(
        const aace::engine::core::ServiceDescription& description,
        const aace::engine::core::Version minRequiredVersion,
        std::initializer_list<std::string> interfaceList);

    bool isInterfaceEnabled(const std::string& name);

    // aace::core::EngineService
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool configure() override;

    // configure the aasb interface
    virtual bool configureAASBInterface(const std::string& name, bool enabled, std::istream& configuration);

public:
    virtual ~AASBHandlerEngineService() = default;

private:
    std::unordered_map<std::string, Enablement> m_interfaceMap;
    aace::engine::core::Version m_minRequiredVersion;
};

inline std::ostream& operator<<(std::ostream& stream, const AASBHandlerEngineService::Enablement& enablement) {
    switch (enablement) {
        case AASBHandlerEngineService::Enablement::DEFAULT:
            stream << "DEFAULT";
            break;
        case AASBHandlerEngineService::Enablement::ENABLED:
            stream << "ENABLED";
            break;
        case AASBHandlerEngineService::Enablement::DISABLED:
            stream << "DISABLED";
            break;
    }
    return stream;
}

}  // namespace aasb
}  // namespace engine
}  // namespace aace

#endif
