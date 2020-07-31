/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <algorithm>
#include <iostream>
#include <typeinfo>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/PropertyManager/PropertyManagerEngineService.h"
#include "AACE/Engine/Utils/String/StringUtils.h"

namespace aace {
namespace engine {
namespace propertyManager {

// String to identify log entries originating from this file.
static const std::string TAG("aace.propertyManager.PropertyManagerEngineService");

// register the service
REGISTER_SERVICE(PropertyManagerEngineService);

PropertyManagerEngineService::PropertyManagerEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool PropertyManagerEngineService::initialize() {
    try {
        // register the PropertyManagerServiceInterface
        ThrowIfNot(
            registerServiceInterface<PropertyManagerServiceInterface>(shared_from_this()),
            "registerPropertyManagerServiceInterfaceFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}
bool PropertyManagerEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::propertyManager::PropertyManager>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool PropertyManagerEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::propertyManager::PropertyManager> propertyManager) {
    try {
        ThrowIfNotNull(m_propertyManagerEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the property manager engine implementation
        m_propertyManagerEngineImpl =
            aace::engine::propertyManager::PropertyManagerEngineImpl::create(propertyManager, shared_from_this());
        ThrowIfNull(m_propertyManagerEngineImpl, "createPropertyManagerEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool PropertyManagerEngineService::registerProperty(const PropertyDescription& propertyDescription) {
    try {
        auto name = propertyDescription.getPropertyName();
        ThrowIf(name.empty(), "invalidPropertyName");
        ThrowIf(m_propertyDescriptionMap.find(name) != m_propertyDescriptionMap.end(), "propertyAlreadyRegistered");
        m_propertyDescriptionMap[name] = propertyDescription;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool PropertyManagerEngineService::addListener(
    const std::string& name,
    std::shared_ptr<PropertyListenerInterface> listener) {
    try {
        ThrowIfNull(listener, "nullListener");
        std::lock_guard<std::mutex> lock(m_listenerMutex);
        auto it = m_propertyListenerMap.find(name);
        if (it != m_propertyListenerMap.end()) {
            it->second.insert(listener);
        } else {
            m_propertyListenerMap[name] = {listener};
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("name", name));
        return false;
    }
}

void PropertyManagerEngineService::removeListener(
    const std::string& name,
    std::shared_ptr<PropertyListenerInterface> listener) {
    try {
        ThrowIfNull(listener, "nullListener");
        std::lock_guard<std::mutex> lock(m_listenerMutex);
        auto it = m_propertyListenerMap.find(name);
        ThrowIf(it == m_propertyListenerMap.end(), "propertyNotFound");
        it->second.erase(listener);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("name", name));
    }
}

bool PropertyManagerEngineService::setProperty(
    const std::string& name,
    const std::string& value,
    const bool& fromPlatform) {
    try {
        if (isRunning() == false) {
            AACE_WARN(LX(TAG).d("reason", "setPropertyCalledWhileEngineNotRunning"));
        }
        ThrowIf(name.empty(), "invalidPropertyName");
        auto it = m_propertyDescriptionMap.find(name);
        ThrowIf(it == m_propertyDescriptionMap.end(), "propertyNotFound");
        ThrowIfNull(it->second.setter(), "readOnlyProperty");
        auto setterResult = m_executor.submit([this, name, value, it, fromPlatform] {
            try {
                bool changed = false;
                bool async = false;
                auto callback = [fromPlatform, this](
                                    const std::string& name, const std::string& value, const std::string& state) {
                    setPropertyResultCallback(name, value, fromPlatform, state);
                };
                auto result = it->second.setter()(value, changed, async, callback);
                ReturnIf(result && async, true);
                if (m_propertyManagerEngineImpl == nullptr) {
                    AACE_WARN(
                        LX(TAG).m("Null propertyManagerEngineImpl. PropertyManager platform interface not registered"));
                } else {
                    result ? handleSetSuccess(changed, fromPlatform, name, value)
                           : handleSetFailed(fromPlatform, name, value);
                }

                return result;
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG).d("reason", ex.what()));
                return false;
            }
        });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("name", name).sensitive("value", value));
        return false;
    }
}

void PropertyManagerEngineService::handleSetSuccess(
    const bool& changed,
    const bool& fromPlatform,
    const std::string& name,
    const std::string& value) {
    // Check if the property value has changed.
    if (changed) {
        // If setProperty() was initiated by the platform, call the
        // propertyStateChanged(name, value, SUCCEEDED), else call the
        // handlePropertyChanged(name, value)
        if (fromPlatform) {
            m_propertyManagerEngineImpl->propertyStateChanged(
                name, value, aace::propertyManager::PropertyManagerEngineInterface::PropertyState::SUCCEEDED);
        } else {
            m_propertyManagerEngineImpl->handlePropertyChanged(name, value);
        }
        // notify the listeners of the property change irrespective of the initiator of the
        // setProperty()
        notifyPropertyChangeListeners(name, value);
    } else {  // The property value did not change
        // If setProperty() was initiated by the platform, call the
        // propertyStateChanged(name, value, SUCCEEDED)
        if (fromPlatform) {
            m_propertyManagerEngineImpl->propertyStateChanged(
                name, value, aace::propertyManager::PropertyManagerEngineInterface::PropertyState::SUCCEEDED);
        }
    }
}

void PropertyManagerEngineService::handleSetFailed(
    const bool& fromPlatform,
    const std::string& name,
    const std::string& value) {
    // If setProperty() was initiated by the platform, call the
    // propertyStateChanged(name, value, FAILED)
    if (fromPlatform) {
        m_propertyManagerEngineImpl->propertyStateChanged(
            name, value, aace::propertyManager::PropertyManagerEngineInterface::PropertyState::FAILED);
    }
}

void PropertyManagerEngineService::setPropertyResultCallback(
    const std::string& name,
    const std::string& value,
    const bool& fromPlatform,
    const std::string& result) {
    m_executor.submit([this, name, value, fromPlatform, result] {
        if (m_propertyManagerEngineImpl == nullptr) {
            AACE_WARN(LX(TAG).m("PropertyManager platform interface not registered"));
        } else {
            aace::engine::utils::string::equal(result, "SUCCEEDED") ? handleSetSuccess(true, fromPlatform, name, value)
                                                                    : handleSetFailed(fromPlatform, name, value);
        }
    });
}

std::string PropertyManagerEngineService::getProperty(const std::string& name) {
    try {
        if (isRunning() == false) {
            AACE_WARN(LX(TAG).d("reason", "getPropertyCalledWhileEngineNotRunning"));
        }
        ThrowIf(name.empty(), "invalidPropertyName");
        auto it = m_propertyDescriptionMap.find(name);
        ThrowIf(it == m_propertyDescriptionMap.end(), "propertyNotFound");
        return it->second.getter()();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("name", name));
        return "";
    }
}

void PropertyManagerEngineService::notifyPropertyChangeListeners(
    const std::string& name,
    const std::string& propertyValue) {
    try {
        std::unique_lock<std::mutex> lock(m_listenerMutex);
        ThrowIf(name.empty(), "invalidPropertyName");
        auto it = m_propertyListenerMap.find(name);
        if (it != m_propertyListenerMap.end()) {
            auto currentListeners = it->second;
            lock.unlock();
            for (auto& listener : currentListeners) {
                listener->propertyChanged(name, propertyValue);
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("name", name).sensitive("value", propertyValue));
    }
}

void PropertyManagerEngineService::updatePropertyValue(const std::string& name, const std::string& newValue) {
    try {
        if (isRunning() == false) {
            AACE_WARN(LX(TAG).d("reason", "setPropertyCalledWhileEngineNotRunning"));
        }
        ThrowIf(name.empty(), "invalidPropertyName");
        auto it = m_propertyDescriptionMap.find(name);
        if (it != m_propertyDescriptionMap.end()) {
            auto propertyValue = it->second.getter()();
            notifyPropertyChangeListeners(name, propertyValue);
            if (m_propertyManagerEngineImpl != nullptr) {
                m_propertyManagerEngineImpl->handlePropertyChanged(name, propertyValue);
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("name", name).sensitive("value", newValue));
    }
}

bool PropertyManagerEngineService::shutdown() {
    if (m_propertyManagerEngineImpl != nullptr) {
        m_propertyManagerEngineImpl->shutdown();
        m_propertyManagerEngineImpl.reset();
    }
    m_executor.shutdown();
    m_propertyListenerMap.clear();
    m_propertyDescriptionMap.clear();
    return true;
}

}  // namespace propertyManager
}  // namespace engine
}  // namespace aace
