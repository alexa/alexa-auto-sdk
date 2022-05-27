/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_AASB_AASB_ENGINE_SERVICE_H
#define AACE_ENGINE_AASB_AASB_ENGINE_SERVICE_H

#include <AACE/Engine/Core/EngineService.h>
#include <AACE/Engine/MessageBroker/MessageBrokerEngineService.h>
#include <AACE/AASB/AASB.h>

#include "AASBEngineImpl.h"

namespace aace {
namespace engine {
namespace aasb {

class AASBEngineService
        : public aace::engine::core::EngineService
        , public std::enable_shared_from_this<AASBEngineService> {
public:
    DESCRIBE("aace.aasb", VERSION("1.0"), DEPENDS(aace::engine::messageBroker::MessageBrokerEngineService))

private:
    AASBEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~AASBEngineService() = default;

protected:
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool start() override;
    bool stop() override;

    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::aasb::AASB> aasb);

private:
    std::shared_ptr<aace::engine::aasb::AASBEngineImpl> m_aasbEngineImpl;
};

}  // namespace aasb
}  // namespace engine
}  // namespace aace

#endif
