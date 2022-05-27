/*
 * Copyright 2017-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CORE_ENGINE_SERVICE_H
#define AACE_ENGINE_CORE_ENGINE_SERVICE_H

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <istream>
#include <typeindex>
#include <typeinfo>
#include <functional>

#include <iostream>

#include "AACE/Engine/Core/ServiceDescription.h"
#include "AACE/Core/PlatformInterface.h"

namespace aace {
namespace engine {
namespace core {

class EngineImpl;
class EngineContext;
class ServiceDescription;

static const std::string DEFAULT_SERVICE_FACTORY_ID = "__default__";

class EngineService {
protected:
    EngineService(const aace::engine::core::ServiceDescription& description);

public:
    using ServiceFactory = std::function<std::shared_ptr<void>()>;

public:
    virtual ~EngineService();

    bool isRunning();

    const ServiceDescription& getDescription();

    template <class T>
    bool registerServiceFactory(ServiceFactory fn, const std::string& id = DEFAULT_SERVICE_FACTORY_ID) {
        auto key = typeid(T).name();
        auto outer_iterator = m_serviceFactoryMap.find(key);
        if (outer_iterator == m_serviceFactoryMap.end()) {
            std::unordered_map<std::string, ServiceFactory> innerMap({{id, fn}});
            m_serviceFactoryMap[key] = innerMap;
            return true;
        } else {
            auto inner_iterator = m_serviceFactoryMap[key].find(id);
            if (inner_iterator == m_serviceFactoryMap[key].end()) {
                m_serviceFactoryMap[key][id] = fn;
                return true;
            }
        }
        return false;
    }

    template <class T>
    std::shared_ptr<T> getServiceInterface() {
        auto key = typeid(T).name();
        auto it = m_serviceInterfaceMap.find(key);
        return it != m_serviceInterfaceMap.end() ? std::static_pointer_cast<T>(it->second.lock()) : nullptr;
    }

protected:
    virtual bool initialize();
    virtual bool configure();
    virtual bool configure(std::shared_ptr<std::istream> configuration);
    virtual bool preRegister();
    virtual bool postRegister();
    virtual bool setup();
    virtual bool start();
    virtual bool stop();
    virtual bool shutdown();
    virtual bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface);
    virtual bool engineStarted();
    virtual bool engineStopped();

    std::shared_ptr<aace::engine::core::EngineContext> getContext();

    template <class T>
    std::shared_ptr<T> newFactoryInstance(
        ServiceFactory defaultFactory,
        const std::string& id = DEFAULT_SERVICE_FACTORY_ID) {
        auto key = typeid(T).name();
        auto outer_iterator = m_serviceFactoryMap.find(key);
        if (outer_iterator != m_serviceFactoryMap.end()) {
            auto inner_iterator = m_serviceFactoryMap[key].find(id);
            if (inner_iterator != m_serviceFactoryMap[key].end()) {
                return std::static_pointer_cast<T>(inner_iterator->second());
            }
        }
        return std::static_pointer_cast<T>(defaultFactory());
    }

    template <class T>
    std::vector<std::shared_ptr<T>> getFactoryType() {
        std::vector<std::shared_ptr<T>> factoryList;
        auto key = typeid(T).name();
        if (m_serviceFactoryMap.find(key) != m_serviceFactoryMap.end()) {
            for (auto it = m_serviceFactoryMap[key].begin(); it != m_serviceFactoryMap[key].end(); it++) {
                factoryList.push_back(std::static_pointer_cast<T>(it->second()));
            }
        }
        return factoryList;
    }

    template <class T>
    bool registerServiceInterface(std::shared_ptr<T> serviceInterface) {
        auto key = typeid(T).name();
        if (m_serviceInterfaceMap.find(key) == m_serviceInterfaceMap.end()) {
            m_serviceInterfaceMap[key] = serviceInterface;
            return true;
        } else {
            return false;
        }
    }

private:
    bool handleInitializeEngineEvent(std::shared_ptr<aace::engine::core::EngineContext> context);
    bool handleConfigureEngineEvent(std::shared_ptr<std::istream> configuration);
    bool handlePreRegisterEngineEvent();
    bool handlePostRegisterEngineEvent();
    bool handleSetupEngineEvent();
    bool handleStartEngineEvent();
    bool handleStopEngineEvent();
    bool handleShutdownEngineEvent();
    bool handleRegisterPlatformInterfaceEngineEvent(std::shared_ptr<aace::core::PlatformInterface> platformInterface);
    bool handleEngineStartedEngineEvent();
    bool handleEngineStoppedEngineEvent();

private:
    std::shared_ptr<aace::engine::core::EngineContext> m_context;

    ServiceDescription m_description;

    bool m_initialized;
    bool m_running;

    // service factory map
    std::unordered_map<std::string, std::unordered_map<std::string, ServiceFactory>> m_serviceFactoryMap;

    // service interface map
    std::unordered_map<std::string, std::weak_ptr<void>> m_serviceInterfaceMap;

    // allow the EngineImpl call private functions in this class
    friend class aace::engine::core::EngineImpl;
};

//
// EngineServiceContext
//

class EngineServiceContext {
public:
    EngineServiceContext(std::shared_ptr<EngineService> service) : m_service(service){};

    template <class T>
    bool registerServiceFactory(EngineService::ServiceFactory fn, const std::string& id = DEFAULT_SERVICE_FACTORY_ID) {
        return m_service->registerServiceFactory<T>(fn, id);
    }

    template <class T>
    std::shared_ptr<T> getServiceInterface() {
        return m_service->getServiceInterface<T>();
    }

private:
    std::shared_ptr<EngineService> m_service;
};

//
// EngineContext
//

class EngineContext {
public:
    virtual ~EngineContext();

    virtual bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) = 0;

    template <class T>
    std::shared_ptr<EngineServiceContext> getService() {
        return getService(T::getServiceDescription().getType());
    }

    virtual std::shared_ptr<EngineServiceContext> getService(const std::string& type) = 0;

    template <class T>
    std::shared_ptr<T> getServiceInterface(const std::string& serviceType) {
        auto service = getService(serviceType);
        return service != nullptr ? service->getServiceInterface<T>() : nullptr;
    }
};

}  // namespace core
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CORE_ENGINE_SERVICE_H
