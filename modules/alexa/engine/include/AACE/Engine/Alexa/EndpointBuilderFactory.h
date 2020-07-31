/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ENDPOINT_BUILDER_FACTORY_H
#define AACE_ENGINE_ENDPOINT_BUILDER_FACTORY_H

#include <AVSCommon/SDKInterfaces/Endpoints/EndpointRegistrationManagerInterface.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <Alexa/AlexaInterfaceMessageSender.h>
#include <Endpoints/EndpointBuilder.h>

#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

/**
 * Creates @c alexaClientSDK::endpoints::EndpointBuilder instances for configuring new endpoints to be controlled by 
 * the device
 */
class EndpointBuilderFactory : public alexaClientSDK::avsCommon::utils::RequiresShutdown {
public:
    /**
     * Creates an EndpointBuilderFactory
     *
     * @param deviceInfo Structure with information about the Alexa client device
     * @param endpointRegistrationManager Object responsible for registering a new endpoint
     * @param contextManager Object used to retrieve the current state of an endpoint
     * @param exceptionSender Object used to send exceptions
     * @param alexaInterfaceMessageSender Object used to send AlexaInterface events
     * @return A pointer to the new endpoint builder if successful; otherwise @c nullptr
     */
    static std::shared_ptr<EndpointBuilderFactory> create(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointRegistrationManagerInterface>
            endpointRegistrationManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceMessageSender> alexaMessageSender);

    /**
     * Creates an endpoint builder to configure a new endpoint to be controlled by the device.
     *
     * @return A pointer to the new endpoint builder if successful; otherwise @c nullptr
     * @warning All endpoints must be built by the time you call @c Engine::start(). Building new endpoints after the
     * client has been connected will fail.
     */
    std::unique_ptr<alexaClientSDK::endpoints::EndpointBuilder> createEndpointBuilder();

protected:
    /// RequiresShutdown
    /// @{
    void doShutdown() override;
    /// @}
private:
    /**
     * Constructor
     *
     * @param deviceInfo Structure with information about the Alexa client device
     * @param endpointRegistrationManager Object responsible for registering a new endpoint
     * @param contextManager Object used to retrieve the current state of an endpoint
     * @param exceptionSender Object used to send exceptions
     * @param alexaInterfaceMessageSender Object used to send AlexaInterface events
     */
    EndpointBuilderFactory(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointRegistrationManagerInterface>
            endpointRegistrationManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceMessageSender> alexaMessageSender);

    /// Information about the Alexa client device
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
    /// Registers new endpoints
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointRegistrationManagerInterface>
        m_endpointManager;
    /// Retrieves the state of an endpoint
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;
    /// Sends exceptions
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> m_exceptionSender;
    /// Sends AlexaInterface events
    std::shared_ptr<alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceMessageSender> m_alexaMessageSender;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace
#endif
