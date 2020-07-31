/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Alexa/WakewordEngineManager.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

static const std::string TAG("aace.alexa.WakewordEngineManager");

bool WakewordEngineManager::registerFactory(const std::string& name, WakewordEngineAdapterFactory factory) {
    if (m_factoryMap.find(name) == m_factoryMap.end()) {
        m_factoryMap[name] = factory;
        return true;
    } else {
        return false;
    }
}

std::shared_ptr<WakewordEngineAdapter> WakewordEngineManager::createAdapter(
    const AdapterType& type,
    const std::string& name) {
    auto it = m_factoryMap.begin();
    if (it == m_factoryMap.end()) {
        AACE_ERROR(LX(TAG, "No factory has been registered."));
        return nullptr;
    }

    if (name == "") {
        AACE_INFO(LX(TAG, "Using default factory").d("name", it->first));
    } else {
        it = m_factoryMap.find(name);
        if (it == m_factoryMap.end()) {
            AACE_ERROR(LX(TAG, "Factory not found").d("name", name));
            return nullptr;
        }
    }

    return it->second(type);
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
