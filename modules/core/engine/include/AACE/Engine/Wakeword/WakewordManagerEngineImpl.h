/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_ENGINEIMPL_H_
#define AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_ENGINEIMPL_H_

#include <memory>

#include "AACE/Wakeword/WakewordManager.h"
#include <AACE/Wakeword/WakewordManagerEngineInterface.h>
#include <AACE/Engine/Wakeword/WakewordManagerServiceInterface.h>

namespace aace {
namespace engine {
namespace wakeword {

class WakewordManagerEngineImpl
        : public aace::wakeword::WakewordManagerEngineInterface
        , public std::enable_shared_from_this<WakewordManagerEngineImpl> {
private:
    /**
     * Constructor.
     */
    WakewordManagerEngineImpl(std::shared_ptr<aace::wakeword::WakewordManager> wakewordManagerPlatformInterface);

public:
    using WakeWordIndex = aace::wakeword::WakewordManagerEngineInterface::WakeWordIndex;

    /**
     * Factory method for creating instance of @c  WakewordManagerEngineImpl
     */
    static std::shared_ptr<WakewordManagerEngineImpl> create(
        std::shared_ptr<aace::wakeword::WakewordManager> wakewordManagerPlatformInterface);

    virtual ~WakewordManagerEngineImpl() = default;

    void register3PWakewordManagerDelegate(
        std::shared_ptr<aace::engine::wakeword::WakewordManagerDelegateInterface> wakewordManagerDelegate);

    /// @name   Functions
    /// @{
    void handleOnWakewordDetected(const std::string& wakeword, WakeWordIndex beginIndex, WakeWordIndex endIndex);
    bool onEnable3PWakeword(const std::string& wakeword, bool wakewordState) override;

    /// @}

    void doShutdown();

private:
    /// Auto SDK WakewordManager platform interface handler instance.
    std::shared_ptr<aace::wakeword::WakewordManager> m_wakewordManagerPlatformInterface;

    std::weak_ptr<aace::engine::wakeword::WakewordManagerDelegateInterface> m_wakewordManagerDelegate;
};

}  // namespace wakeword
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_ENGINEIMPL_H_
