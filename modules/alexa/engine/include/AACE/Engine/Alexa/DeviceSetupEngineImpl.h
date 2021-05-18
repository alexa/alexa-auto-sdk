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

#ifndef AACE_ENGINE_ALEXA_DEVICESETUPENGINEIMPL_H_
#define AACE_ENGINE_ALEXA_DEVICESETUPENGINEIMPL_H_

#include <memory>

#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <DeviceSetup/DeviceSetup.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/DeviceSetup.h"

namespace aace {
namespace engine {
namespace alexa {

/**
 * This handles the instantiation of the @c DeviceSetup and the interaction 
 * with it to send events. 
 */
class DeviceSetupEngineImpl
        : public aace::alexa::DeviceSetupEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<DeviceSetupEngineImpl> {
private:
    /**
     * Constructor.
     */
    DeviceSetupEngineImpl(
        std::shared_ptr<aace::alexa::DeviceSetup> deviceSetupPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

    /**
     * Initialize the @c DeviceSetupEngineImpl and @c DeviceSetupCapabilityAgent.
     */
    bool initialize();

public:
    /**
     * Factory method for creating instance of @c DeviceSetupEngineImpl which handles
     * instantiation of @c DeviceSetupCapabilityAgent.
     */
    static std::shared_ptr<DeviceSetupEngineImpl> create(
        std::shared_ptr<aace::alexa::DeviceSetup> deviceSetupPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

    /// @name DeviceSetupEngineInterface Functions
    /// @{
    /** 
     * This function sends out an event sendDeviceSetupComplete() of capability agent @c DeviceSetup
    */
    void onSetupCompleted() override;
    /// @}

protected:
    /// @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

private:
    /// Auto SDK Alexa DeviceSetup platform interface handler instance.
    std::shared_ptr<aace::alexa::DeviceSetup> m_deviceSetupPlatformInterface;

    /// The device setup capability agent
    std::shared_ptr<alexaClientSDK::capabilityAgents::deviceSetup::DeviceSetup> m_deviceSetupCapabilityAgent;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_DEVICESETUPENGINEIMPL_H_
