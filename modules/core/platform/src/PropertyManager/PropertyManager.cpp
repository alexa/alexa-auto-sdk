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

#include "AACE/PropertyManager/PropertyManager.h"

namespace aace {
namespace propertyManager {

PropertyManager::~PropertyManager() = default;

bool PropertyManager::setProperty(const std::string& name, const std::string& value) {
    return m_propertyManagerEngineInterface != nullptr ? m_propertyManagerEngineInterface->onSetProperty(name, value)
                                                       : false;
}

std::string PropertyManager::getProperty(const std::string& name) {
    return m_propertyManagerEngineInterface != nullptr ? m_propertyManagerEngineInterface->onGetProperty(name) : "";
}

void PropertyManager::setEngineInterface(
    std::shared_ptr<PropertyManagerEngineInterface> propertyManagerEngineInterface) {
    m_propertyManagerEngineInterface = propertyManagerEngineInterface;
}

}  // namespace propertyManager
}  // namespace aace
