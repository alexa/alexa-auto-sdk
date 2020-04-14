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

#ifndef AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_LISTENER_INTERFACE_H
#define AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_LISTENER_INTERFACE_H

namespace aace {
namespace engine {
namespace propertyManager {

/**
 * The PropertyListenerInterface notifies listeners about a change in the
 * value of a property. The property must be registered using
 * aace::engine::propertyManager::PropertyManagerServiceInterface::registerProperty()
 *
 */
class PropertyListenerInterface {
public:
    /**
     * Notifies the listener about a property value change.
     * @note The listener should return immediately from this method.
     *
     * @param [in] name The name used by the Engine to identify the property.
     *        This name must be one of the property constants recognized
     *        by the Engine; for example, the properties in
     *        @c aace::alexa::property::AlexaProperties.h
     * @param [in] newValue The new value of the property
     */
    virtual void propertyChanged(const std::string& name, const std::string& newValue) = 0;
};

}  // namespace propertyManager
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_PROPERTY_MANAGER_PROPERTY_LISTENER_INTERFACE_H
