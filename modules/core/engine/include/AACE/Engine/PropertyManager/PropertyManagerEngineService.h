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

#ifndef AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_MANAGER_ENGINE_SERVICE_H
#define AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_MANAGER_ENGINE_SERVICE_H

#include <AVSCommon/Utils/Threading/Executor.h>

#include "AACE/Engine/Core/EngineService.h"
#include "AACE/PropertyManager/PropertyManager.h"
#include "PropertyDescription.h"
#include "PropertyManagerEngineImpl.h"
#include "PropertyManagerServiceInterface.h"

namespace aace {
namespace engine {
namespace propertyManager {

class PropertyManagerEngineService
        : public aace::engine::core::EngineService
        , public PropertyManagerServiceInterface
        , public std::enable_shared_from_this<PropertyManagerEngineService> {
public:
    DESCRIBE("aace.propertyManager", VERSION("1.0"))

private:
    PropertyManagerEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~PropertyManagerEngineService() = default;

    // PropertyManagerServiceInterface
    virtual bool registerProperty(const PropertyDescription& propertyDescription) override;
    virtual void updatePropertyValue(const std::string& name, const std::string& newValue) override;
    virtual bool addListener(const std::string& name, std::shared_ptr<PropertyListenerInterface> listener) override;
    virtual void removeListener(const std::string& name, std::shared_ptr<PropertyListenerInterface> listener) override;
    virtual bool setProperty(const std::string& name, const std::string& value, const bool& fromPlatform) override;
    virtual std::string getProperty(const std::string& name) override;

    // Callback function to notify the PropertyManagerEngineService the result
    // of setProperty() operation.
    void setPropertyResultCallback(
        const std::string& name,
        const std::string& value,
        const bool& fromPlatform,
        const std::string& result);

protected:
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;
    bool initialize() override;
    bool shutdown() override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::propertyManager::PropertyManager> propertyManager);

    // Notifies all the listeners about the property change by calling
    // propertyChanged() on every PropertyListenerInterface.
    void notifyPropertyChangeListeners(const std::string& key, const std::string& propertyValue);

    // Notifies the platform and listeners about a successful set property
    // operation. Expects m_propertyManagerEngineImpl to be not null.
    void handleSetSuccess(
        const bool& changed,
        const bool& fromPlatform,
        const std::string& name,
        const std::string& value);

    // Notifies the platform about a failed set property operation
    // Expects m_propertyManagerEngineImpl to be not null.
    void handleSetFailed(const bool& fromPlatform, const std::string& name, const std::string& value);

private:
    // Map to store property name and the ProperteryDescription object
    // associated with that property.
    std::unordered_map<std::string, PropertyDescription> m_propertyDescriptionMap;

    // Map to store property name and the set of listeners for that property.
    // The property owner is responsible for adding itself as a listener to the
    // property upon which it depends.
    std::unordered_map<std::string, std::unordered_set<std::shared_ptr<PropertyListenerInterface>>>
        m_propertyListenerMap;

    std::mutex m_listenerMutex;
    std::shared_ptr<PropertyManagerEngineImpl> m_propertyManagerEngineImpl;

    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

}  // namespace propertyManager
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_MANAGER_ENGINE_SERVICE_H
