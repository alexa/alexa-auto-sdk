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

#include <AACE/Engine/Core/EngineMacros.h>

#include "AACE/Engine/DeviceUsage/DeviceUsageEngineService.h"

namespace aace {
namespace engine {
namespace deviceUsage {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.deviceUsage.DeviceUsageEngineService");

/// Register the service.
REGISTER_SERVICE(DeviceUsageEngineService);

DeviceUsageEngineService::DeviceUsageEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService{description} {
}

bool DeviceUsageEngineService::initialize() {
    try {
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool DeviceUsageEngineService::shutdown() {
    AACE_INFO(LX(TAG));
    if (m_deviceUsageEngineImpl != nullptr) {
        m_deviceUsageEngineImpl->doShutdown();
        m_deviceUsageEngineImpl.reset();
    }
    return true;
}

bool DeviceUsageEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::deviceUsage::DeviceUsage>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool DeviceUsageEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::deviceUsage::DeviceUsage> deviceUsagePlatformInterface) {
    AACE_INFO(LX(TAG).m("Registering DeviceUsage platform interface"));
    try {
        ThrowIfNotNull(m_deviceUsageEngineImpl, "platformInterfaceAlreadyRegistered");

        m_deviceUsageEngineImpl =
            aace::engine::deviceUsage::DeviceUsageEngineImpl::create(deviceUsagePlatformInterface);
        ThrowIfNull(m_deviceUsageEngineImpl, "createDeviceUsageEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace deviceUsage
}  // namespace engine
}  // namespace aace
