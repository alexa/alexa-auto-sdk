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

#ifndef AASB_ENGINE_CONNECTIVITY_AASB_CONNECTIVITY_ENGINE_SERVICE_H
#define AASB_ENGINE_CONNECTIVITY_AASB_CONNECTIVITY_ENGINE_SERVICE_H

#include <AACE/Engine/AASB/AASBEngineService.h>
#include <AACE/Engine/AASB/AASBHandlerEngineService.h>

namespace aasb {
namespace engine {
namespace connectivity {

class AASBConnectivityEngineService : public aace::engine::aasb::AASBHandlerEngineService {
public:
    DESCRIBE("aasb.connectivity", VERSION("1.0"), DEPENDS(aace::engine::aasb::AASBEngineService))

private:
    AASBConnectivityEngineService(const aace::engine::core::ServiceDescription& description);

protected:
    bool postRegister() override;

public:
    virtual ~AASBConnectivityEngineService() = default;
};

}  // namespace connectivity
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_CONNECTIVITY_AASB_CONNECTIVITY_ENGINE_SERVICE_H
