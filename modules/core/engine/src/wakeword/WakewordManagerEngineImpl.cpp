/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <sstream>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Wakeword/WakewordManagerEngineImpl.h>
#include <AACE/Engine/Utils/Metrics/Metrics.h>
#include <AACE/Engine/Wakeword/WakewordManagerServiceInterface.h>

namespace aace {
namespace engine {
namespace wakeword {

using namespace aace::engine::utils::metrics;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.wakeword.WakewordManagerEngineImpl");

WakewordManagerEngineImpl::WakewordManagerEngineImpl(
    std::shared_ptr<aace::wakeword::WakewordManager> wakewordManagerPlatformInterface) :
        m_wakewordManagerPlatformInterface{wakewordManagerPlatformInterface} {
}

std::shared_ptr<WakewordManagerEngineImpl> WakewordManagerEngineImpl::create(
    std::shared_ptr<aace::wakeword::WakewordManager> wakewordManagerPlatformInterface) {
    try {
        ThrowIfNull(wakewordManagerPlatformInterface, "invalidWakewordManagernPlatformInterface");
        //Initialize wakeword adpater pass through for Alexa
        auto wakewordManagerEngineImpl =
            std::shared_ptr<WakewordManagerEngineImpl>(new WakewordManagerEngineImpl(wakewordManagerPlatformInterface));

        // set the platform engine interface reference
        wakewordManagerPlatformInterface->setEngineInterface(wakewordManagerEngineImpl);
        return wakewordManagerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void WakewordManagerEngineImpl::register3PWakewordManagerDelegate(
    std::shared_ptr<aace::engine::wakeword::WakewordManagerDelegateInterface> wakewordManagerDelegate) {
    try {
        m_wakewordManagerDelegate = wakewordManagerDelegate;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void WakewordManagerEngineImpl::doShutdown() {
    if (m_wakewordManagerPlatformInterface != nullptr) {
        m_wakewordManagerPlatformInterface->setEngineInterface(nullptr);
        m_wakewordManagerPlatformInterface.reset();
    }
}

void WakewordManagerEngineImpl::handleOnWakewordDetected(
    const std::string& name,
    WakeWordIndex beginIndex,
    WakeWordIndex endIndex) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIfNull(m_wakewordManagerPlatformInterface, "nullWakewordManagerPlatformInterface");
        m_wakewordManagerPlatformInterface->onWakewordDetected(name, beginIndex, endIndex);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool WakewordManagerEngineImpl::onEnable3PWakeword(const std::string& wakeword, bool wakewordState) {
    AACE_DEBUG(LX(TAG));
    try {
        auto wakewordManagerDelegate_lock = m_wakewordManagerDelegate.lock();
        ThrowIfNull(wakewordManagerDelegate_lock, "invalidwakewordManagerDelegatInstance");
        ThrowIfNot(
            wakewordManagerDelegate_lock->enable3PWakewordDetection(wakeword, wakewordState), "enable3PWakewordFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}
}  // namespace wakeword
}  // namespace engine
}  // namespace aace