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

#ifndef AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_DESCRIPTION_H
#define AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_DESCRIPTION_H

#include <functional>
#include <string>

namespace aace {
namespace engine {
namespace propertyManager {

/**
 * The PropertyDescription class is used by any module that owns a property to
 * encapsulate property details such as name, setter
 * functions, and getter functions.
 */
class PropertyDescription {
public:
    /**
     * Function used to get the property value. For example, for locale,
     * the PropertyDescription function retrieves the
     * @c aace::alexa::property::AlexaProperties::LOCALE property value.
     */
    using Getter = std::function<std::string()>;

    /**
     * Callback function to notify the PropertyManagerEngineService of the
     * result of setProperty() operation.
     */
    using SetterCallback = std::function<void(const std::string&, const std::string&, const std::string&)>;

    /**
     * Function used to set the property value. For example, for locale,
     * the PropertyDescription function sets the
     * @c aace::alexa::property::AlexaProperties::LOCALE property value.
     *
     * @param [in] std::string The property setting
     * @param [out] bool Flag set to @c true by the module that owns the
     *        property to indicate whether the property value was updated
     * @param [out] bool Flag set to @c true by the setter to indicate whether
     *        the setter returns asynchronously
     * @return @c true if the property value was updated or set to the current
     *         setting, else @c false if an error occured.
     */
    using Setter = std::function<bool(const std::string&, bool&, bool&, const SetterCallback&)>;

    PropertyDescription() = default;
    PropertyDescription(const std::string& name, Setter setter, Getter getter);
    PropertyDescription(const PropertyDescription& other);
    PropertyDescription& operator=(const PropertyDescription& other) = default;

    Getter getter() const;
    Setter setter() const;
    std::string getPropertyName() const;

private:
    Setter m_setter;
    Getter m_getter;
    std::string m_name;
};

}  // namespace propertyManager
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_DESCRIPTION_H
