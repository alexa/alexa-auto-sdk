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

#ifndef AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_ENGINE_SERVICE_H
#define AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_ENGINE_SERVICE_H

#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/CustomDomain/CustomDomain.h"
#include "CustomDomainEngineImpl.h"

namespace aace {
namespace engine {
namespace customDomain {

class CustomDomainEngineService : public aace::engine::core::EngineService {
    DESCRIBE("aace.customDomain", VERSION("1.0"), DEPENDS(aace::engine::alexa::AlexaEngineService))

private:
    CustomDomainEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~CustomDomainEngineService() = default;

protected:
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool shutdown() override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::customDomain::CustomDomain> customDomain);

    /// The engine implementation instance
    std::shared_ptr<aace::engine::customDomain::CustomDomainEngineImpl> m_customDomainEngineImpl;

    // Capability metadata from engine config
    std::string m_customInterfaceMetadata;
};

}  // namespace customDomain
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_ENGINE_SERVICE_H
