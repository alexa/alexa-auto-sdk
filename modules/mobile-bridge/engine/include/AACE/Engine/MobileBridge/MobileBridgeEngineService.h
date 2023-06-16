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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_MOBILE_BRIDGE_ENGINE_SERVICE_H
#define AACE_ENGINE_MOBILE_BRIDGE_MOBILE_BRIDGE_ENGINE_SERVICE_H

#include <memory>

#include "AACE/Engine/Core/EngineService.h"
#include "AACE/Engine/MobileBridge/Config.h"
#include "AACE/Engine/MobileBridge/MobileBridgeEngineImpl.h"
#include "AACE/MobileBridge/MobileBridge.h"

namespace aace {
namespace engine {
namespace mobileBridge {

class MobileBridgeEngineService
        : public core::EngineService
        , public std::enable_shared_from_this<MobileBridgeEngineService> {
    DESCRIBE("aace.mobileBridge", VERSION("1.0"))

public:
    explicit MobileBridgeEngineService(const core::ServiceDescription& description);
    ~MobileBridgeEngineService() override = default;

    bool configure(std::shared_ptr<std::istream> configuration) override;

protected:
    bool setup() override;
    bool start() override;
    bool stop() override;
    bool shutdown() override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr && registerPlatformInterfaceType(typedPlatformInterface);
    }
    bool registerPlatformInterfaceType(std::shared_ptr<aace::mobileBridge::MobileBridge> transportProvider);

    std::shared_ptr<MobileBridgeEngineImpl> m_engineImpl;
    std::shared_ptr<Config> m_config;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif
