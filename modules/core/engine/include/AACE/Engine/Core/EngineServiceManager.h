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

#ifndef AACE_ENGINE_CORE_ENGINE_SERVICE_MANAGER_H
#define AACE_ENGINE_CORE_ENGINE_SERVICE_MANAGER_H

#include <functional>

#include "EngineService.h"

namespace aace {
namespace engine {
namespace core {

class ServiceFactory;

class EngineServiceManager {
private:
    EngineServiceManager() = default;

    // private static accessor
    static std::shared_ptr<EngineServiceManager> getInstance();

    // declare service map type
    using ServiceMap = std::unordered_map<std::string, std::shared_ptr<ServiceFactory>>;

public:
    static bool registerService(
        const ServiceDescription& description,
        std::function<std::shared_ptr<EngineService>(const ServiceDescription&)> constructor);
    static EngineServiceManager::ServiceMap::iterator registryBegin();
    static EngineServiceManager::ServiceMap::iterator registryEnd();

private:
    ServiceMap m_serviceMap;

    // singleton
    static std::shared_ptr<EngineServiceManager> s_instance;
};

//
// ServiceFactory
//

class ServiceFactory {
private:
    ServiceFactory() = default;

public:
    static std::shared_ptr<ServiceFactory> create(
        const ServiceDescription& description,
        std::function<std::shared_ptr<EngineService>(const ServiceDescription&)> constructor);

    std::shared_ptr<EngineService> newInstance();

    const ServiceDescription& getDescription() {
        return m_description;
    }

private:
    ServiceDescription m_description;
    std::function<std::shared_ptr<EngineService>(const ServiceDescription&)> m_constructor;
};

}  // namespace core
}  // namespace engine
}  // namespace aace

// EngineServiceManager Macros
#define REGISTER_SERVICE(T)                                                                       \
    const bool T::SERVICE_REGISTERED_VAR_NAME = []() -> bool {                                    \
        return aace::engine::core::EngineServiceManager::registerService(                         \
            T::getServiceDescription(),                                                           \
            [](const aace::engine::core::ServiceDescription& description) -> std::shared_ptr<T> { \
                return std::shared_ptr<T>(new T(description));                                    \
            });                                                                                   \
    }();

#endif  // AACE_ENGINE_CORE_ENGINE_SERVICE_MANAGER_H
