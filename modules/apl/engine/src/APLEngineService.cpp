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

#include <memory>

#include <AACE/Engine/Core/EngineMacros.h>

#include "AACE/Engine/APL/APLEngineImpl.h"
#include "AACE/Engine/APL/APLEngineService.h"

namespace aace {
namespace engine {
namespace apl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.apl.APLEngineService");

// register the service
REGISTER_SERVICE(APLEngineService);

APLEngineService::APLEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool APLEngineService::start() {
    return true;
}

bool APLEngineService::stop() {
    return true;
}

bool APLEngineService::shutdown() {
    if (m_aplEngineImpl != nullptr) {
        m_aplEngineImpl->shutdown();
        m_aplEngineImpl.reset();
    }
    return true;
}

bool APLEngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::apl::APL>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool APLEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::apl::APL> apl) {
    try {
        ThrowIfNull(apl, "platformInterfaceNull");
        ThrowIfNotNull(m_aplEngineImpl, "platformInterfaceAlreadyRegistered");

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

        auto focusManager = alexaComponents->getVisualFocusManager();
        ThrowIfNull(focusManager, "focusManagerInvalid");

        auto dialogUXStateAggregator = alexaComponents->getDialogUXStateAggregator();
        ThrowIfNull(dialogUXStateAggregator, "dialogUXStateAggregatorInvalid");

        m_aplEngineImpl = aace::engine::apl::APLEngineImpl::create(
            apl,
            defaultEndpointBuilder,
            focusManager,
            exceptionSender,
            messageSender,
            contextManager,
            dialogUXStateAggregator);
        ThrowIfNull(m_aplEngineImpl, "createAPLEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<APL>").d("reason", ex.what()));
        return false;
    }
}

}  // namespace apl
}  // namespace engine
}  // namespace aace
