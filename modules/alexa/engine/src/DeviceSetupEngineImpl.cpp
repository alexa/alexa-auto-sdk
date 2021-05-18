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
#include "AACE/Engine/Alexa/DeviceSetupEngineImpl.h"

namespace aace {
namespace engine {
namespace alexa {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.DeviceSetupEngineImpl");

DeviceSetupEngineImpl::DeviceSetupEngineImpl(
    std::shared_ptr<aace::alexa::DeviceSetup> deviceSetupPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown{TAG},
        m_deviceSetupPlatformInterface{deviceSetupPlatformInterface},
        m_messageSender(messageSender) {
}

bool DeviceSetupEngineImpl::initialize() {
    AACE_INFO(LX(TAG));
    try {
        m_deviceSetupCapabilityAgent =
            alexaClientSDK::capabilityAgents::deviceSetup::DeviceSetup::create(m_messageSender);
        ThrowIfNull(m_deviceSetupCapabilityAgent, "couldNotCreateCapabilityAgent");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<DeviceSetupEngineImpl> DeviceSetupEngineImpl::create(
    std::shared_ptr<aace::alexa::DeviceSetup> deviceSetupPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender) {
    AACE_INFO(LX(TAG));
    try {
        ThrowIfNull(deviceSetupPlatformInterface, "invalidPlatformInterface");
        ThrowIfNull(messageSender, "invalidMessageSender");

        auto deviceSetupEngineImpl = std::shared_ptr<DeviceSetupEngineImpl>(
            new DeviceSetupEngineImpl(deviceSetupPlatformInterface, messageSender));

        ThrowIfNot(deviceSetupEngineImpl->initialize(), "initializeDeviceSetupEngineImplFailed");

        // Set the platform engine interface reference.
        deviceSetupPlatformInterface->setEngineInterface(deviceSetupEngineImpl);

        return deviceSetupEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void DeviceSetupEngineImpl::onSetupCompleted() {
    AACE_INFO(LX(TAG));
    try {
        auto deviceSetupCapabilityAgent = m_deviceSetupCapabilityAgent;
        auto deviceSetupPlatformInterface = m_deviceSetupPlatformInterface;
        m_executor.submit([deviceSetupCapabilityAgent, deviceSetupPlatformInterface] {
            auto resultFuture = deviceSetupCapabilityAgent->sendDeviceSetupComplete(
                alexaClientSDK::interfaces::deviceSetup::AssistedSetup::NONE);
            if ((resultFuture.wait_for(std::chrono::milliseconds(1000)) == std::future_status::ready)) {
                auto result = resultFuture.get();
                deviceSetupPlatformInterface->setupCompletedResponse(
                    result ? aace::alexa::DeviceSetup::StatusCode::SUCCESS
                           : aace::alexa::DeviceSetup::StatusCode::FAIL);
            } else {
                deviceSetupPlatformInterface->setupCompletedResponse(aace::alexa::DeviceSetup::StatusCode::FAIL);
            }
        });
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_deviceSetupPlatformInterface->setupCompletedResponse(aace::alexa::DeviceSetup::StatusCode::FAIL);
    }
}

void DeviceSetupEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));
    if (m_deviceSetupPlatformInterface != nullptr) {
        m_deviceSetupPlatformInterface->setEngineInterface(nullptr);
        m_deviceSetupPlatformInterface.reset();
    }
    m_executor.shutdown();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
