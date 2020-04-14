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

#ifndef AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_MANAGER_ENGINE_IMPL_H
#define AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_MANAGER_ENGINE_IMPL_H

#include <unordered_map>
#include <unordered_set>

#include <AVSCommon/Utils/RequiresShutdown.h>

#include "AACE/PropertyManager/PropertyManager.h"
#include "AACE/PropertyManager/PropertyManagerEngineInterface.h"
#include "PropertyManagerServiceInterface.h"

namespace aace {
namespace engine {
namespace propertyManager {

class PropertyManagerEngineImpl
        : public aace::propertyManager::PropertyManagerEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown {
private:
    PropertyManagerEngineImpl(std::shared_ptr<aace::propertyManager::PropertyManager> platformPropertyManagerInterface);

    bool initialize(std::shared_ptr<PropertyManagerServiceInterface> propertyManagerServiceInterface);

protected:
    void doShutdown() override;

public:
    static std::shared_ptr<PropertyManagerEngineImpl> create(
        std::shared_ptr<aace::propertyManager::PropertyManager> platformPropertyManagerInterface,
        std::shared_ptr<PropertyManagerServiceInterface> propertyManagerServiceInterface);

    // PropertyManagerEngineInterface
    virtual bool onSetProperty(const std::string& name, const std::string& value) override;
    virtual std::string onGetProperty(const std::string& name) override;

    /**
     * Called by the module that owns the property to notify the
     * PropertyManagerEngineImpl that a property value has changed.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        This name must be one of the property constants recognized
     *        by the Engine; for example, the properties in
     *        @c aace::alexa::property::AlexaProperties.h
     * @param [in] newValue The new property value.
     */
    void handlePropertyChanged(const std::string& name, const std::string& value);

    /**
     * Called by the PropertyManagerEngineService to notify the
     * PropertyManagerEngineImpl of a property state change.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        This name must be one of the property constants recognized
     *        by the Engine; for example, the properties in
     *        @c aace::alexa::property::AlexaProperties.h
     * @param [in] value The property value.
     * @param [in] state The property state.
     */
    void propertyStateChanged(
        const std::string& name,
        const std::string& value,
        const aace::propertyManager::PropertyManagerEngineInterface::PropertyState state);

private:
    std::shared_ptr<aace::propertyManager::PropertyManager> m_platformPropertyManagerInterface;

    std::weak_ptr<PropertyManagerServiceInterface> m_propertyManagerServiceInterface;
};

}  // namespace propertyManager
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_MANAGER_ENGINE_IMPL_H
