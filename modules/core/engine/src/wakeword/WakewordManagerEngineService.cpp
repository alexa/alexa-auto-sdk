/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <nlohmann/json.hpp>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/JSON/JSON.h"
#include "AACE/Engine/Utils/String/StringUtils.h"

#include "AACE/Engine/Wakeword/WakewordManagerEngineService.h"

namespace aace {
namespace engine {
namespace wakeword {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.wakeword.WakewordManagerEngineService");
using namespace aace::engine::utils::string;

using json = nlohmann::json;

/// String constants in configuration
static const std::string WAKEWORDMANAGER = "WakewordManager";
static const std::string THIRD_PARTY_WAKEWORDS = "3PWakewords";

/// Register the service.
REGISTER_SERVICE(WakewordManagerEngineService);

WakewordManagerEngineService::WakewordManagerEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService{description} {
}

bool WakewordManagerEngineService::initialize() {
    try {
        // register the WakewordManagerServiceInterface
        ThrowIfNot(
            registerServiceInterface<WakewordManagerServiceInterface>(shared_from_this()),
            "registerWakewordManagerServiceInterfaceFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool WakewordManagerEngineService::shutdown() {
    AACE_INFO(LX(TAG));
    if (m_wakewordManagerEngineImpl != nullptr) {
        m_wakewordManagerEngineImpl->doShutdown();
        m_wakewordManagerEngineImpl.reset();
    }
    return true;
}

bool WakewordManagerEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::wakeword::WakewordManager>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool WakewordManagerEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::wakeword::WakewordManager> wakewordManagerPlatformInterface) {
    AACE_INFO(LX(TAG).m("Registering WakewordManager platform interface"));
    try {
        ThrowIfNotNull(m_wakewordManagerEngineImpl, "platformInterfaceAlreadyRegistered");

        m_wakewordManagerEngineImpl =
            aace::engine::wakeword::WakewordManagerEngineImpl::create(wakewordManagerPlatformInterface);
        ThrowIfNull(m_wakewordManagerEngineImpl, "createWakewordEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool WakewordManagerEngineService::register3PWakewordManagerDelegate(
    std::shared_ptr<aace::engine::wakeword::WakewordManagerDelegateInterface> wakewordManagerDelegate) {
    try {
        ThrowIfNull(wakewordManagerDelegate, "wakewordManagerDelegateIsNull");
        ThrowIfNull(m_wakewordManagerEngineImpl, "wakewordManagerEngineImplIsNull");

        m_wakewordManagerEngineImpl->register3PWakewordManagerDelegate(wakewordManagerDelegate);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void WakewordManagerEngineService::updateOnWakewordDetected(
    const std::string& wakeword,
    WakeWordIndex beginIndex,
    WakeWordIndex endIndex) {
    try {
        if (isRunning() == false) {
            AACE_WARN(LX(TAG).d("reason", "onWakewordDetectedWhileEngineNotRunning"));
        }
        ThrowIfNull(m_wakewordManagerEngineImpl, "wakewordManagerEngineImplIsNull");
        m_wakewordManagerEngineImpl->handleOnWakewordDetected(wakeword, beginIndex, endIndex);

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("wakeword", wakeword).sensitive("beginIndex", beginIndex));
    }
}

bool WakewordManagerEngineService::configure(std::shared_ptr<std::istream> configuration) {
    AACE_INFO(LX(TAG));
    try {
        json jconfiguration;
        jconfiguration = nlohmann::json::parse(*configuration);
        auto& wakewordManager = jconfiguration.at(WAKEWORDMANAGER);
        ThrowIfNot(
            wakewordManager.contains(THIRD_PARTY_WAKEWORDS), "no3PWakewordsConfiguration");
        auto thirdPartyWakewords = wakewordManager[THIRD_PARTY_WAKEWORDS];
        if (thirdPartyWakewords != nullptr) {
            ThrowIfNot(thirdPartyWakewords.is_array(), "invalid3PWakewordsConfiguration");
            for (auto next : thirdPartyWakewords) {
                AACE_INFO(LX(TAG).d("3PWakeword", next));
                m_wakewordConfig.insert(toUpper(next));
            }
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}

std::unordered_set<std::string> WakewordManagerEngineService::getConfigured3PWakewords() {
    AACE_INFO(LX(TAG));
    return m_wakewordConfig;
}

}  // namespace wakeword
}  // namespace engine
}  // namespace aace