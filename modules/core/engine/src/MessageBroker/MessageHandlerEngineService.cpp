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

#include <AACE/Engine/MessageBroker/MessageHandlerEngineService.h>
#include <AACE/Engine/MessageBroker/MessageBrokerServiceInterface.h>
#include <AACE/Engine/MessageBroker/MessageBrokerEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace messageBroker {

// String to identify log entries originating from this file.
static const std::string TAG("aace.messageBroker.MessageHandlerEngineService");

MessageHandlerEngineService::MessageHandlerEngineService(
    const aace::engine::core::ServiceDescription& description,
    const aace::engine::core::Version minRequiredVersion,
    std::initializer_list<std::string> interfaceList,
    std::initializer_list<std::string> defaultEnabledInterfaceList) :
        aace::engine::core::EngineService(description), m_minRequiredVersion(minRequiredVersion) {
    // legacy aasb interfaces should default to message broker auto enablement configuration
    for (auto& next : interfaceList) {
        m_interfaceMap[next] = Enablement::DEFAULT;
    }

    // new interfaces define after 4.0 should always be enabled by default,
    // unless explicitly disabled in the interface configuration
    for (auto& next : defaultEnabledInterfaceList) {
        m_interfaceMap[next] = Enablement::DEFAULT_ENABLED;
    }
}

bool MessageHandlerEngineService::isInterfaceEnabled(const std::string& name) {
    try {
        auto it = m_interfaceMap.find(name);
        ThrowIf(it == m_interfaceMap.end(), "invalidInterfaceName");

        auto messageServiceInterface =
            getContext()->getServiceInterface<aace::engine::messageBroker::MessageBrokerServiceInterface>(
                "aace.messageBroker");
        ThrowIfNull(messageServiceInterface, "invalidMessageBrokerServiceInterface");

        bool enabled = false;

        // get the interface enablement based on configured settings
        switch (it->second) {
            case Enablement::DEFAULT:
                enabled = messageServiceInterface->getAutoEnableInterfaces();
                break;
            case Enablement::DEFAULT_ENABLED:
            case Enablement::ENABLED:
                enabled = true;
                break;
            case Enablement::DISABLED:
                enabled = false;
                break;
        }

        AACE_DEBUG(LX(TAG).d("interface", name).d("enabled", enabled).d("enablementConfiguration", it->second));

        return enabled;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MessageHandlerEngineService::configure() {
    try {
        auto messageServiceInterface =
            getContext()->getServiceInterface<aace::engine::messageBroker::MessageBrokerServiceInterface>(
                "aace.messageBroker");
        ThrowIfNull(messageServiceInterface, "invalidMessageBrokerServiceInterface");

        // get the  current and configured versions from the message service interface
        auto currentVersion = messageServiceInterface->getCurrentVersion();
        auto configuredVersion = messageServiceInterface->getConfiguredVersion();

        // validate the configured version is greater than the min required version and is
        // less than or equal to the current aasb message version
        ReturnIfNot(configuredVersion < m_minRequiredVersion || configuredVersion > currentVersion, true);

        // log the version error message
        AACE_ERROR(LX(TAG)
                       .d("reason", "invalidConfiguredVersion")
                       .d("required", m_minRequiredVersion.toString())
                       .d("configured", configuredVersion.toString())
                       .d("current", currentVersion.toString()));

        return false;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MessageHandlerEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        // default configuration
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
                configureMessageInterface(next.first, isInterfaceEnabled(next.first), interfaceConfigStream);
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MessageHandlerEngineService::configureMessageInterface(
    const std::string& name,
    bool enabled,
    std::istream& configuration) {
    AACE_DEBUG(LX(TAG).d("name", name).d("enabled", enabled));
    return true;
}

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace
