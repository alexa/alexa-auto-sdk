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

#include <nlohmann/json.hpp>
#include <sstream>

#include <AACE/Engine/AASB/AASBHandlerEngineService.h>
#include <AACE/Engine/AASB/AASBServiceInterface.h>
#include <AACE/Engine/AASB/AASBEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace aasb {

// String to identify log entries originating from this file.
static const std::string TAG("aace.aasb.AASBHandlerEngineService");

AASBHandlerEngineService::AASBHandlerEngineService(
    const aace::engine::core::ServiceDescription& description,
    const aace::engine::core::Version minRequiredVersion,
    std::initializer_list<std::string> interfaceList) :
        aace::engine::core::EngineService(description) {
    for (auto& next : interfaceList) {
        m_interfaceMap[next] = Enablement::DEFAULT;
    }

    m_minRequiredVersion = minRequiredVersion;
}

bool AASBHandlerEngineService::isInterfaceEnabled(const std::string& name) {
    try {
        auto it = m_interfaceMap.find(name);
        ThrowIf(it == m_interfaceMap.end(), "invalidInterfaceName");

        auto aasbServiceInterface =
            getContext()->getServiceInterface<aace::engine::aasb::AASBServiceInterface>("aace.aasb");
        ThrowIfNull(aasbServiceInterface, "invalidAASBServiceInterface");

        AACE_DEBUG(LX(TAG)
                       .d("interface", name)
                       .d("enablement", it->second)
                       .d("autoEnabled", aasbServiceInterface->getAutoEnableInterfaces()));

        return it->second == Enablement::DEFAULT ? aasbServiceInterface->getAutoEnableInterfaces()
                                                 : it->second == Enablement::ENABLED;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBHandlerEngineService::configure() {
    try {
        auto aasbServiceInterface =
            getContext()->getServiceInterface<aace::engine::aasb::AASBServiceInterface>("aace.aasb");
        ThrowIfNull(aasbServiceInterface, "invalidAASBServiceInterface");

        aace::engine::core::Version configuredVersion = aasbServiceInterface->getConfiguredVersion();
        ThrowIfNot(
            (m_minRequiredVersion < configuredVersion || m_minRequiredVersion == configuredVersion) &&
                (configuredVersion < aasbServiceInterface->getCurrentVersion() ||
                 configuredVersion == aasbServiceInterface->getCurrentVersion()),
            "invalidConfiguredVersion");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBHandlerEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        // version check
        ThrowIfNot(configure(), "configureFailed");

        // process the service configuration
        auto root = nlohmann::json::parse(*configuration);

        for (auto& next : m_interfaceMap) {
            auto interfaceRoot = root[nlohmann::json::json_pointer("/" + next.first)];

            if (interfaceRoot != nullptr) {
                auto enabledNode = interfaceRoot["/enabled"_json_pointer];

                // set the configured enabled state, or use default if not explicitly configured
                if (enabledNode != nullptr && enabledNode.is_boolean()) {
                    next.second = enabledNode.get<bool>() ? Enablement::ENABLED : Enablement::DISABLED;
                }

                // configure the interface
                std::stringstream interfaceConfigStream;

                // generate the config stream for the interface
                interfaceConfigStream << interfaceRoot.dump();

                // call service configure interface method
                configureAASBInterface(next.first, isInterfaceEnabled(next.first), interfaceConfigStream);
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBHandlerEngineService::configureAASBInterface(
    const std::string& name,
    bool enabled,
    std::istream& configuration) {
    AACE_DEBUG(LX(TAG).d("name", name).d("enabled", enabled));
    return true;
}

}  // namespace aasb
}  // namespace engine
}  // namespace aace
