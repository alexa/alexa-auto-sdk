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

#include <AACE/Engine/Core/EngineMacros.h>
#include <nlohmann/json.hpp>

#include "AACE/Engine/Connectivity/ConnectivityConstants.h"
#include "AACE/Engine/Connectivity/ConnectivityEngineService.h"

namespace aace {
namespace engine {
namespace connectivity {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.connectivity.ConnectivityEngineService");

/// Register the service.
REGISTER_SERVICE(ConnectivityEngineService);

ConnectivityEngineService::ConnectivityEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService{description} {
}

bool ConnectivityEngineService::shutdown() {
    AACE_INFO(LX(TAG));
    if (m_alexaConnectivityEngineImpl != nullptr) {
        m_alexaConnectivityEngineImpl->shutdown();
        m_alexaConnectivityEngineImpl.reset();
    }
    return true;
}

bool ConnectivityEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::connectivity::AlexaConnectivity>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool ConnectivityEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::connectivity::AlexaConnectivity> alexaConnectivity) {
    AACE_INFO(LX(TAG).m("Registering AlexaConnectivity platform interface"));
    try {
        ThrowIfNotNull(m_alexaConnectivityEngineImpl, "platformInterfaceAlreadyRegistered");

        auto alexaComponents =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "alexaComponentsInvalid");

        auto defaultEndpointBuilder = alexaComponents->getDefaultEndpointBuilder();
        ThrowIfNull(defaultEndpointBuilder, "defaultEndpointBuilderInvalid");

        auto contextManager = alexaComponents->getContextManager();
        ThrowIfNull(contextManager, "contextManagerInvalid");

        m_alexaConnectivityEngineImpl = aace::engine::connectivity::AlexaConnectivityEngineImpl::create(
            alexaConnectivity, defaultEndpointBuilder, contextManager, m_networkIdentifier);
        ThrowIfNull(m_alexaConnectivityEngineImpl, "createAlexaConnectivityEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<Connectivity>").d("reason", ex.what()));
        return false;
    }
}

}  // namespace connectivity
}  // namespace engine
}  // namespace aace
