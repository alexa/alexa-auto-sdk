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

#ifndef AACE_ENGINE_DEVICEUSAGE_DEVICEUSAGEENGINESERVICE_H_
#define AACE_ENGINE_DEVICEUSAGE_DEVICEUSAGEENGINESERVICE_H_

#include "AACE/Engine/Core/EngineService.h"
#include "AACE/DeviceUsage/DeviceUsage.h"
#include "AACE/Engine/DeviceUsage/DeviceUsageEngineImpl.h"

namespace aace {
namespace engine {
namespace deviceUsage {

class DeviceUsageEngineService : public aace::engine::core::EngineService {
public:
    DESCRIBE("aace.deviceUsage", VERSION("1.0"))

private:
    DeviceUsageEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~DeviceUsageEngineService() = default;

protected:
    bool initialize() override;
    bool shutdown() override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    /// Platform interface registration.
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::deviceUsage::DeviceUsage> deviceUsagePlatformInterface);

    /// Engine implementation object references.
    std::shared_ptr<aace::engine::deviceUsage::DeviceUsageEngineImpl> m_deviceUsageEngineImpl;
};

}  // namespace deviceUsage
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_DEVICEUSAGE_DEVICEUSAGEENGINESERVICE_H_
