/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_SERVICE_INTERFACE_H
#define AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_SERVICE_INTERFACE_H

#include <AACE/Engine/Core/EngineMacros.h>
#include "AACE/Engine/Wakeword/WakewordManagerDelegateInterface.h"
#include <AACE/Wakeword/WakewordManagerEngineInterface.h>

#include <string>
#include <unordered_set>

namespace aace {
namespace engine {
namespace wakeword {
class WakewordManagerServiceInterface {
public:
    using WakeWordIndex = aace::wakeword::WakewordManagerEngineInterface::WakeWordIndex;

    virtual ~WakewordManagerServiceInterface() = default;

    /**
     * @brief Register internal wakeword service interface for a specific wakeword detection support
     * 
     * @param WakewordData 
     * @return true 
     * @return false 
     */
    virtual bool register3PWakewordManagerDelegate(
        std::shared_ptr<aace::engine::wakeword::WakewordManagerDelegateInterface> wakewordManagerDelegate) = 0;

    /**
     * @brief Notified Wakeword engine service that wakeword is detected, internal module will call this to set Wakeword interface
     * 
     * @param wakeword 
     * @param beginIndex 
     * @param endIndex 
     * @return true 
     * @return false 
     */

    virtual void updateOnWakewordDetected(
        const std::string& wakeword,
        WakeWordIndex beginIndex,
        WakeWordIndex endIndex) = 0;

    /**
     * @brief Get the Configured 3P Wakeword Set object
     * 
     * @return std::unordered_set<std::string> wakeword set configured
     */
    virtual std::unordered_set<std::string> getConfigured3PWakewords() = 0;
};

}  // namespace wakeword
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_SERVICE_INTERFACE_H