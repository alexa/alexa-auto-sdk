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

#ifndef AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_ENGINE_SERVICE_H_
#define AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_ENGINE_SERVICE_H_

#include <unordered_set>

#include "AACE/Engine/Core/EngineService.h"

#include "WakewordManagerEngineImpl.h"
#include "WakewordManagerServiceInterface.h"

namespace aace {
namespace engine {
namespace wakeword {

class WakewordManagerEngineService
        : public aace::engine::core::EngineService
        , public WakewordManagerServiceInterface
        , public std::enable_shared_from_this<WakewordManagerEngineService> {
public:
    DESCRIBE("aace.wakewordManager", VERSION("1.0"))
    using WakeWordIndex = aace::wakeword::WakewordManagerEngineInterface::WakeWordIndex;

private:
    WakewordManagerEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~WakewordManagerEngineService() = default;

    /// @name @c WakewordManagerServiceInterface
    /// @{
    bool register3PWakewordManagerDelegate(
        std::shared_ptr<aace::engine::wakeword::WakewordManagerDelegateInterface> wakewordManagerDelegate) override;
    void updateOnWakewordDetected(const std::string& wakeword, WakeWordIndex beginIndex, WakeWordIndex endIndex)
        override;
    virtual std::unordered_set<std::string> getConfigured3PWakewords() override;
    /// @}

protected:
    bool initialize() override;
    bool shutdown() override;
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    /// Platform interface registration.
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(
        std::shared_ptr<aace::wakeword::WakewordManager> wakewordManagerPlatformInterface);

    /// Engine implementation object references.
    std::shared_ptr<WakewordManagerEngineImpl> m_wakewordManagerEngineImpl;
    std::unordered_set<std::string> m_wakewordConfig;
};

}  // namespace wakeword
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_ENGINE_SERVICE_H_
