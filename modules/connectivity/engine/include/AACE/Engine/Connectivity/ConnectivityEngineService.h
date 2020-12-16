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

#ifndef AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_ENGINE_SERVICE_H
#define AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_ENGINE_SERVICE_H

#include <AACE/Engine/Alexa/AlexaEngineService.h>

#include "AACE/Connectivity/AlexaConnectivity.h"
#include "AACE/Engine/Connectivity/AlexaConnectivityEngineImpl.h"

namespace aace {
namespace engine {
namespace connectivity {

class ConnectivityEngineService : public aace::engine::core::EngineService {
    DESCRIBE("aace.connectivity", VERSION("1.0"), DEPENDS(aace::engine::alexa::AlexaEngineService))

private:
    ConnectivityEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~ConnectivityEngineService() = default;

protected:
    bool shutdown() override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    /// Platform interface registration.
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::connectivity::AlexaConnectivity> alexaConnectivity);

    /// The network identifier.
    std::string m_networkIdentifier;

    /// Engine implementation object references.
    std::shared_ptr<aace::engine::connectivity::AlexaConnectivityEngineImpl> m_alexaConnectivityEngineImpl;
};

}  // namespace connectivity
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_ENGINE_SERVICE_H
