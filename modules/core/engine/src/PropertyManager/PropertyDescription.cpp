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

#include "AACE/Engine/PropertyManager/PropertyDescription.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace propertyManager {

// String to identify log entries originating from this file.
static const std::string TAG("aace.core.PropertyDescription");

PropertyDescription::PropertyDescription(const std::string& name, Setter setter, Getter getter) :
        m_setter(setter), m_getter(getter), m_name(name) {
}

PropertyDescription::PropertyDescription(const PropertyDescription& other) {
    *this = other;
}

PropertyDescription::Getter PropertyDescription::getter() const {
    return m_getter;
}

PropertyDescription::Setter PropertyDescription::setter() const {
    return m_setter;
}

std::string PropertyDescription::getPropertyName() const {
    return m_name;
}

}  // namespace propertyManager
}  // namespace engine
}  // namespace aace
