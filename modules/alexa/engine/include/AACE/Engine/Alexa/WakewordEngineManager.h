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

#ifndef AACE_ENGINE_ALEXA_INTERFACE_WAKEWORD_ENGINE_MANAGER_H
#define AACE_ENGINE_ALEXA_INTERFACE_WAKEWORD_ENGINE_MANAGER_H

#include <functional>
#include <memory>
#include <unordered_map>

#include "WakewordEngineAdapter.h"

namespace aace {
namespace engine {
namespace alexa {

class WakewordEngineManager {
public:
    enum class AdapterType { PRIMARY, SECONDARY };

    using WakewordEngineAdapterFactory = std::function<std::shared_ptr<WakewordEngineAdapter>(const AdapterType& type)>;

    WakewordEngineManager() = default;
    ~WakewordEngineManager() = default;

    /**
     * Register the factory method for the @c WakewordEngineAdapter
     *
     * @param name The unique name for identifying the wake-word engine
     * @param factory The factory method to create @c WakewordEngineAdapter instance
     * 
     * @return returns @c true on successful registration, otherwise @false.
     * 
     */
    bool registerFactory(const std::string& name, WakewordEngineAdapterFactory factory);

    /**
     * Create the @c WakewordEngineAdapter
     *
     * @param type The type of the wake-word engine
     * @param name Optional string to identify the wake-word engine, default to empty
     * 
     * @return returns @c true on successful registration, otherwise @false.
     * 
     */
    std::shared_ptr<WakewordEngineAdapter> createAdapter(const AdapterType& type, const std::string& name = "");

private:
    std::unordered_map<std::string, WakewordEngineAdapterFactory> m_factoryMap;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_INTERFACE_WAKEWORD_ENGINE_MANAGER_H
