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

#include "AACE/Engine/Alexa/EndpointBuilderFactory.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

/// String to identify log entries originating from this file
static const std::string TAG("EndpointBuilderFactory");

std::shared_ptr<EndpointBuilderFactory> EndpointBuilderFactory::create(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointRegistrationManagerInterface>
        endpointRegistrationManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceMessageSender> alexaMessageSender) {
    try {
        ThrowIfNull(deviceInfo, "nullDeviceInfo");
        ThrowIfNull(endpointRegistrationManager, "nullEndpointRegistrationManager");
        ThrowIfNull(contextManager, "nullContextManager");
        ThrowIfNull(alexaMessageSender, "nullAlexaMessageSender");
        ThrowIfNull(exceptionSender, "nullExceptionSender");

        return std::shared_ptr<EndpointBuilderFactory>(new EndpointBuilderFactory(
            deviceInfo, endpointRegistrationManager, contextManager, exceptionSender, alexaMessageSender));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "createFailed").d("reason", ex.what()));
        return nullptr;
    }
}

EndpointBuilderFactory::EndpointBuilderFactory(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointRegistrationManagerInterface>
        endpointRegistrationManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceMessageSender> alexaMessageSender) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_deviceInfo{deviceInfo},
        m_endpointManager{endpointRegistrationManager},
        m_contextManager{contextManager},
        m_exceptionSender{exceptionSender},
        m_alexaMessageSender{alexaMessageSender} {
}

std::unique_ptr<alexaClientSDK::endpoints::EndpointBuilder> EndpointBuilderFactory::createEndpointBuilder() {
    return alexaClientSDK::endpoints::EndpointBuilder::create(
        *m_deviceInfo, m_endpointManager, m_contextManager, m_exceptionSender, m_alexaMessageSender);
}

void EndpointBuilderFactory::doShutdown() {
    m_endpointManager.reset();
    m_contextManager.reset();
    m_exceptionSender.reset();
    m_alexaMessageSender.reset();
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace