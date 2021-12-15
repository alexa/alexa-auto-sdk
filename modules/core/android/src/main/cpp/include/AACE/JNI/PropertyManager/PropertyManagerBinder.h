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

#ifndef AACE_JNI_PROPERTY_MANAGER_PROPERTY_MANAGER_BINDER_H
#define AACE_JNI_PROPERTY_MANAGER_PROPERTY_MANAGER_BINDER_H

#include <AACE/PropertyManager/PropertyManager.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace propertyManager {

//
// PropertyManagerHandler
//

class PropertyManagerHandler : public aace::propertyManager::PropertyManager {
public:
    PropertyManagerHandler(jobject obj);

    // aace::propertyManager::PropertyManager
    void propertyChanged(const std::string& name, const std::string& newValue) override;
    void propertyStateChanged(const std::string& name, const std::string& value, const PropertyState state) override;

private:
    JObject m_obj;
};

//
// PropertyManagerBinder
//

class PropertyManagerBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    PropertyManagerBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_propertyManagerHandler;
    }

    std::shared_ptr<PropertyManagerHandler> getPropertyManager() {
        return m_propertyManagerHandler;
    }

private:
    std::shared_ptr<PropertyManagerHandler> m_propertyManagerHandler;
};

//
// JPropertyState
//

class JPropertyStateConfig : public EnumConfiguration<PropertyManagerHandler::PropertyState> {
public:
    using T = PropertyManagerHandler::PropertyState;

    const char* getClassName() override {
        return "com/amazon/aace/propertyManager/PropertyManager$PropertyState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::SUCCEEDED, "SUCCEEDED"}, {T::FAILED, "FAILED"}};
    }
};
using JPropertyState = JEnum<PropertyManagerHandler::PropertyState, JPropertyStateConfig>;

}  // namespace propertyManager
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_PROPERTY_MANAGER_PROPERTY_MANAGER_BINDER_H