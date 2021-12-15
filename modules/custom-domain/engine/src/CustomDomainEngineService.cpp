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

#include "AACE/Engine/CustomDomain/CustomDomainEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <nlohmann/json.hpp>

namespace aace {
namespace engine {
namespace customDomain {

using json = nlohmann::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.customDomain.CustomDomainEngineService");

/// Register the service.
REGISTER_SERVICE(CustomDomainEngineService);

CustomDomainEngineService::CustomDomainEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool CustomDomainEngineService::configure(std::shared_ptr<std::istream> configuration) {
    AACE_INFO(LX(TAG));
    try {
        json jconfiguration;
        jconfiguration = json::parse(*configuration);

        m_customInterfaceMetadata = jconfiguration.dump();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool CustomDomainEngineService::shutdown() {
    AACE_INFO(LX(TAG));
    if (m_customDomainEngineImpl != nullptr) {
        m_customDomainEngineImpl->shutdown();
        m_customDomainEngineImpl.reset();
    }
    return true;
}

bool CustomDomainEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    return registerPlatformInterfaceType<aace::customDomain::CustomDomain>(platformInterface);
}

bool CustomDomainEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::customDomain::CustomDomain> customDomain) {
    AACE_INFO(LX(TAG).m("Registering customDomain platform interface"));
    try {
        ThrowIfNotNull(m_customDomainEngineImpl, "platformInterfaceAlreadyRegistered");

        m_customDomainEngineImpl = aace::engine::customDomain::CustomDomainEngineImpl::create(
            customDomain, getContext(), m_customInterfaceMetadata);
        ThrowIfNull(m_customDomainEngineImpl, "createCustomDomainEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<CustomDomain>").d("reason", ex.what()));
        return false;
    }
}

}  // namespace customDomain
}  // namespace engine
}  // namespace aace
