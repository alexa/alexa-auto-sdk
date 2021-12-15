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

#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <acsdkDeviceSetupInterfaces/DeviceSetupInterface.h>

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
    DeviceSetupEngineImpl(std::shared_ptr<aace::alexa::DeviceSetup> deviceSetupPlatformInterface);

    /**
     * Initialize the @c DeviceSetupEngineImpl object.  
     */
    bool initialize(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

public:
    /**
     * Factory method for creating instance of @c DeviceSetupEngineImpl which handles
     * instantiation of @c DeviceSetup.
     */
    static std::shared_ptr<DeviceSetupEngineImpl> create(
        std::shared_ptr<aace::alexa::DeviceSetup> deviceSetupPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

    /// @name DeviceSetupEngineInterface Functions
    /// @{
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
    std::shared_ptr<alexaClientSDK::acsdkDeviceSetupInterfaces::DeviceSetupInterface> m_deviceSetup;

    /// For sending events Alexa
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    /// Executor used to call @c DeviceSetup
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_DEVICESETUPENGINEIMPL_H_
