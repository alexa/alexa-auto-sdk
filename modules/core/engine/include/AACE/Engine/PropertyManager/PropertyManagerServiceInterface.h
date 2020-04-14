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

#ifndef AACE_ENGINE_PROPERTY_PROPERTY_MANAGER_SERVICE_INTERFACE_H
#define AACE_ENGINE_PROPERTY_PROPERTY_MANAGER_SERVICE_INTERFACE_H

#include "PropertyDescription.h"
#include "PropertyListenerInterface.h"

namespace aace {
namespace engine {
namespace propertyManager {

/**
 * PropertyManagerServiceInterface is implemented by the
 * PropertyManagerEngineService to allow the owners of the properties
 * to carry out a set of functions on the PropertyManagerEngineService.
 */
class PropertyManagerServiceInterface {
public:
    virtual ~PropertyManagerServiceInterface() = default;

    /**
     * Registers the property with the Property Manager.
     * @note The property should be registered during the initialize stage of the
     * module or service that owns the property.
     *
     * @param [in] propertyDescription
     *        The aace::engine::propertyManager::PropertyDescription object.
     * @return @c true if the property was registered, else @c false
     *         if an error occured.
     */
    virtual bool registerProperty(const PropertyDescription& propertyDescription) = 0;

    /**
     * Notifies the PropertyManagerEngineService of a property change.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        The property name must be one of the property constants recognized
     *        by the Engine; for example, the properties in
     *        @c aace::alexa::property::AlexaProperties.h
     * @param [in] newValue The updated property value.
     */
    virtual void updatePropertyValue(const std::string& name, const std::string& newValue) = 0;

    /**
     * Adds the @c PropertyListenerInterface as a listener for any change to the
     * property denoted by @c key.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        The property name must be one of the property constants recognized
     *        by the Engine; for example, the properties in
     *        @c aace::alexa::property::AlexaProperties.h
     * @param [in] listener
     *        The @c aace:engine::propertyManager::PropertyListenerInterface
     * @return @c true if the listener value was added, else @c false
     *         if an error occured.
     */
    virtual bool addListener(const std::string& name, std::shared_ptr<PropertyListenerInterface> listener) = 0;

    /**
     * Removes the @c PropertyListenerInterface as a listener for any change to the
     * property denoted by @c key.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        The name must be one of the property constants recognized
     *        by the Engine; for example, the properties in
     *        @c aace::alexa::property::AlexaProperties.h
     * @param [in] listener
     *        The @c aace:engine::propertyManager::PropertyListenerInterface
     */
    virtual void removeListener(const std::string& name, std::shared_ptr<PropertyListenerInterface> listener) = 0;

    /**
     * Sets a property value in the Engine. This can be called by any internal
     * module to set a property.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        The name must be one of the property constants recognized
     *        by the Engine; for example, the properties in
     *        @c aace::alexa::property::AlexaProperties.h
     * @param [in] value The property value.
     * @param [in] fromPlatform Flag to denote if the call to setProperty()
     *        originated from the platform. If @c false, notify the platform via the
     *        @c aace::propertyManager::PropertyManager::propertyChanged().
     * @return @c true if the property value was updated or set to the current
     *         setting, else @c false if an error occured.
     */
    virtual bool setProperty(const std::string& name, const std::string& value, const bool& fromPlatform = false) = 0;

    /**
     * Retrieves the setting for the property identified by
     * @c name from the Engine. This can be called by any internal
     * module to retrieve a property.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        The name must be one of the property constants recognized
     *        by the Engine; for example, the properties in
     *        @c aace::alexa::property::AlexaProperties.h
     * @return The property value as a string, or an empty string if the
     *        property value was not found.
     */
    virtual std::string getProperty(const std::string& name) = 0;
};

}  // namespace propertyManager
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_PROPERTY_PROPERTY_MANAGER_SERVICE_INTERFACE_H
