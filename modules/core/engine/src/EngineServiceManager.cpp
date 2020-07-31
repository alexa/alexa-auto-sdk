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

#include "AACE/Engine/Core/EngineServiceManager.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace core {

// String to identify log entries originating from this file.
static const std::string TAG("aace.core.EngineServiceManager");

std::shared_ptr<EngineServiceManager> EngineServiceManager::s_instance;

std::shared_ptr<EngineServiceManager> EngineServiceManager::getInstance() {
    if (s_instance == nullptr) {
        s_instance = std::shared_ptr<EngineServiceManager>(new EngineServiceManager());
    }

    return s_instance;
}

bool EngineServiceManager::registerService(
    const ServiceDescription& description,
    std::function<std::shared_ptr<EngineService>(const ServiceDescription&)> constructor) {
    try {
        auto type = description.getType();

        ThrowIfNot(
            getInstance()->m_serviceMap.find(type) == getInstance()->m_serviceMap.end(), "serviceAlreadyRegistered");

        getInstance()->m_serviceMap[type] = ServiceFactory::create(description, constructor);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerService").d("reason", ex.what()));
        return false;
    }
}

EngineServiceManager::ServiceMap::iterator EngineServiceManager::registryBegin() {
    return getInstance()->m_serviceMap.begin();
}

EngineServiceManager::ServiceMap::iterator EngineServiceManager::registryEnd() {
    return getInstance()->m_serviceMap.end();
}

//
// ServiceFactory
//

std::shared_ptr<ServiceFactory> ServiceFactory::create(
    const ServiceDescription& description,
    std::function<std::shared_ptr<EngineService>(const ServiceDescription&)> constructor) {
    auto factory = std::shared_ptr<ServiceFactory>(new ServiceFactory());

    factory->m_description = description;
    factory->m_constructor = constructor;

    return factory;
}

std::shared_ptr<EngineService> ServiceFactory::newInstance() {
    return m_constructor(m_description);
}

}  // namespace core
}  // namespace engine
}  // namespace aace
