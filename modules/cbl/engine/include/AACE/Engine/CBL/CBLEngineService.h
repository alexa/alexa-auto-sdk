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

#ifndef AACE_ENGINE_CBL_CBL_ENGINE_SERVICE_H
#define AACE_ENGINE_CBL_CBL_ENGINE_SERVICE_H

#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/CBL/CBL.h"

#include "CBLEngineImpl.h"

namespace aace {
namespace engine {
namespace cbl {

class CBLEngineService : public aace::engine::core::EngineService {
public:
    DESCRIBE("aace.cbl", VERSION("1.0"), DEPENDS(aace::engine::alexa::AlexaEngineService))

private:
    CBLEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~CBLEngineService() = default;

protected:
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool setup() override;
    bool start() override;
    bool stop() override;
    bool shutdown() override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::cbl::CBL> cbl);

private:
    std::shared_ptr<aace::engine::cbl::CBLEngineImpl> m_cblEngineImpl;
    std::chrono::seconds m_codePairRequestTimeout;
    std::string m_endpoint;
    bool m_enableUserProfile;
    std::shared_ptr<CBLAuthorizationProvider> m_cblAuthorizationProvider;
};

}  // namespace cbl
}  // namespace engine
}  // namespace aace

#endif
