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

#ifndef AACE_PROPERTY_MANAGER_PROPERTY_MANAGER_H
#define AACE_PROPERTY_MANAGER_PROPERTY_MANAGER_H

#include <iostream>
#include <string>
#include "AACE/Core/PlatformInterface.h"
#include "PropertyManagerEngineInterface.h"

/** @file */

namespace aace {
namespace propertyManager {

/**
 * PropertyManager should be extended to set and retrieve Engine property
 * settings and be notified of property changes.
 *
 * @sa aace::alexa::property::AlexaProperties.h
 * @sa aace::core::property::CoreProperties.h
 * @sa aace::network::property::NetworkProperties.h
 * @sa aace::vehicle::property::VehicleProperties.h
 */
class PropertyManager : public aace::core::PlatformInterface {
protected:
    PropertyManager() = default;

public:
    virtual ~PropertyManager();

    using PropertyState = aace::propertyManager::PropertyManagerEngineInterface::PropertyState;
    /**
     * Sets a property value in the Engine. setProperty() is an asynchronous
     * operation and the Engine will call propertyStateChanged() with the status
     * when it is completed.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        The property name must be one of the property constants recognized
     *        by the Engine, e.g. the properties in
     *        @c aace::alexa::property::AlexaProperties.h.
     * @param [in] value The property setting
     * @return @c true if the property value was updated or set to the current
     *         setting, else @c false if an error occured.
     */
    bool setProperty(const std::string& name, const std::string& value);

    /**
     * Notifies the platform implementation of the status of a property change
     * after a call to setProperty().
     *
     * @param [in] name The name used by the Engine to identify the property.
     * @param [in] value The property value.
     * @param [in] state The state of the property change.
     */
    virtual void propertyStateChanged(const std::string& name, const std::string& value, const PropertyState state) = 0;

    /**
     * Retrieves the setting for the property identified by @c name from the
     * Engine.
     *
     * @note After calling setProperty(), getProperty() will return the updated
     * value only after the Engine calls propertyStateChanged() with
     * PropertyState::SUCCEEDED.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        The property name must be one of the property constants recognized
     *        by the Engine, e.g. the properties in
     *        @c aace::alexa::property::AlexaProperties.h
     * @return The property value as a string, or an empty string if the
     *        property value was not found
     */
    std::string getProperty(const std::string& name);

    /**
     * Notifies the platform implementation of a property setting change in the
     * Engine.
     * @note This will not be called if the property setting change was
     * initiated by @c PropertyManager::setProperty()
     *
     * @param [in] name The name used by the Engine to identify the property.
     * @param [in] newValue The new value of the property
     */
    virtual void propertyChanged(const std::string& name, const std::string& newValue) = 0;

    /**
     * @internal
     * Sets the Engine interface delagate
     *
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface(std::shared_ptr<PropertyManagerEngineInterface> propertyManagerEngineInterface);

private:
    std::shared_ptr<PropertyManagerEngineInterface> m_propertyManagerEngineInterface;
};

inline std::ostream& operator<<(std::ostream& stream, const PropertyManager::PropertyState& state) {
    switch (state) {
        case PropertyManager::PropertyState::SUCCEEDED:
            stream << "SUCCEEDED";
            break;
        case PropertyManager::PropertyState::FAILED:
            stream << "FAILED";
            break;
    }
    return stream;
}

}  // namespace propertyManager
}  // namespace aace

#endif  // AACE_PROPERTY_MANAGER_PROPERTY_MANAGER_H
