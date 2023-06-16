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

#include "AACE/Engine/MobileBridge/MobileBridgeEngineService.h"

#include <memory>
#include <utility>

#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/MobileBridge/MobileBridge.h"
#include "nlohmann/json.hpp"

namespace aace {
namespace engine {
namespace mobileBridge {

// String to identify log entries originating from this file.
static const char* TAG = "MobileBridgeEngineService";

// NOLINTNEXTLINE(cert-err58-cpp)
REGISTER_SERVICE(MobileBridgeEngineService)

MobileBridgeEngineService::MobileBridgeEngineService(const core::ServiceDescription& description) :
        core::EngineService(description) {
}

bool MobileBridgeEngineService::configure(std::shared_ptr<std::istream> configuration) {
    AACE_INFO(LX(TAG));

    auto j = nlohmann::json::parse(*configuration);
    m_config = std::make_shared<Config>();
    from_json(j, *m_config);

    return true;
}

bool MobileBridgeEngineService::setup() {
    AACE_INFO(LX(TAG));
    try {
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
    return false;
}

bool MobileBridgeEngineService::start() {
    AACE_INFO(LX(TAG));
    return true;
}

bool MobileBridgeEngineService::stop() {
    AACE_INFO(LX(TAG));
    return true;
}

bool MobileBridgeEngineService::shutdown() {
    AACE_INFO(LX(TAG));
    return true;
}

bool MobileBridgeEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::mobileBridge::MobileBridge>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool MobileBridgeEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::mobileBridge::MobileBridge> mobileBridge) {
    try {
        ThrowIfNotNull(m_engineImpl, "platformInterfaceAlreadyRegistered");

        auto alexaComponents =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "invalidAlexaComponentInterface");

        auto deviceInfo = alexaComponents->getDeviceInfo();
        ThrowIfNull(deviceInfo, "deviceInfoInvalid");

        ThrowIfNull(mobileBridge, "invalidPlatformInterface");
        m_engineImpl = MobileBridgeEngineImpl::create(mobileBridge, deviceInfo);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("interface", "aace::mobileBridge::MobileBridge").d("reason", ex.what()));
    }
    return false;
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
