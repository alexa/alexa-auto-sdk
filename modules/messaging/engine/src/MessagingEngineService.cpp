/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Messaging/MessagingEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace messaging {

// String to identify log entries originating from this file.
static const std::string TAG("aace.messaging.MessagingEngineService");

// register the service
REGISTER_SERVICE(MessagingEngineService);

MessagingEngineService::MessagingEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool MessagingEngineService::start() {
    AACE_INFO(LX(TAG));
    return true;
}

bool MessagingEngineService::stop() {
    AACE_INFO(LX(TAG));
    return true;
}

bool MessagingEngineService::shutdown() {
    AACE_INFO(LX(TAG));
    if (m_messagingEngineImpl != nullptr) {
        m_messagingEngineImpl->shutdown();
        m_messagingEngineImpl.reset();
    }
    return true;
}

bool MessagingEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::messaging::Messaging>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool MessagingEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::messaging::Messaging> messaging) {
    AACE_INFO(LX(TAG).m("Registering messaging platform interface"));
    try {
        ThrowIfNotNull(m_messagingEngineImpl, "platformInterfaceAlreadyRegistered");

        auto alexaComponents =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "invalidAlexaComponentInterface");

        auto defaultEndpointBuilder = alexaComponents->getDefaultEndpointBuilder();
        ThrowIfNull(defaultEndpointBuilder, "defaultEndpointBuilderInvalid");

        auto exceptionSender = alexaComponents->getExceptionEncounteredSender();
        ThrowIfNull(exceptionSender, "exceptionSenderInvalid");

        auto messageSender = alexaComponents->getMessageSender();
        ThrowIfNull(messageSender, "messageSenderInvalid");

        auto contextManager = alexaComponents->getContextManager();
        ThrowIfNull(contextManager, "contextManagerInvalid");

        m_messagingEngineImpl = aace::engine::messaging::MessagingEngineImpl::create(
            messaging, defaultEndpointBuilder, exceptionSender, contextManager, messageSender);
        ThrowIfNull(m_messagingEngineImpl, "createMessagingEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<Messaging>").d("reason", ex.what()));
        return false;
    }
}

}  // namespace messaging
}  // namespace engine
}  // namespace aace
