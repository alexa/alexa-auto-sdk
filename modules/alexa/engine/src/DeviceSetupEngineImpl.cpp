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

#include <acsdkDeviceSetup/DeviceSetupFactory.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include "AACE/Engine/Alexa/DeviceSetupEngineImpl.h"

namespace aace {
namespace engine {
namespace alexa {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.DeviceSetupEngineImpl");

DeviceSetupEngineImpl::DeviceSetupEngineImpl(std::shared_ptr<aace::alexa::DeviceSetup> deviceSetupPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown{TAG},
        m_deviceSetupPlatformInterface{deviceSetupPlatformInterface} {
}

bool DeviceSetupEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender) {
    AACE_INFO(LX(TAG));
    try {
        auto deviceSetupInterfaces = alexaClientSDK::acsdkDeviceSetup::createDeviceSetup(messageSender);
        ThrowIfNot(deviceSetupInterfaces.hasValue(), "couldNotCreateDeviceSetupInstance");

        m_deviceSetup = deviceSetupInterfaces.value().deviceSetup;

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

        auto deviceSetupEngineImpl =
            std::shared_ptr<DeviceSetupEngineImpl>(new DeviceSetupEngineImpl(deviceSetupPlatformInterface));

        ThrowIfNot(deviceSetupEngineImpl->initialize(messageSender), "initializeDeviceSetupEngineImplFailed");

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

    auto deviceSetup = m_deviceSetup;
    auto deviceSetupPlatformInterface = m_deviceSetupPlatformInterface;
    m_executor.submit([deviceSetup, deviceSetupPlatformInterface] {
        auto resultFuture =
            deviceSetup->sendDeviceSetupComplete(alexaClientSDK::acsdkDeviceSetupInterfaces::AssistedSetup::NONE);
        if ((resultFuture.wait_for(std::chrono::milliseconds(1000)) == std::future_status::ready)) {
            auto result = resultFuture.get();
            deviceSetupPlatformInterface->setupCompletedResponse(
                result ? aace::alexa::DeviceSetup::StatusCode::SUCCESS : aace::alexa::DeviceSetup::StatusCode::FAIL);
        } else {
            AACE_ERROR(LX(TAG).m("sendDeviceSetupCompleteTimedOut"));
            deviceSetupPlatformInterface->setupCompletedResponse(aace::alexa::DeviceSetup::StatusCode::FAIL);
        }
    });
}

void DeviceSetupEngineImpl::doShutdown() {
    AACE_INFO(LX(TAG));
    m_executor.shutdown();
    if (m_deviceSetupPlatformInterface != nullptr) {
        m_deviceSetupPlatformInterface->setEngineInterface(nullptr);
        m_deviceSetupPlatformInterface.reset();
    }
    if (m_deviceSetup != nullptr) {
        m_deviceSetup.reset();
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
