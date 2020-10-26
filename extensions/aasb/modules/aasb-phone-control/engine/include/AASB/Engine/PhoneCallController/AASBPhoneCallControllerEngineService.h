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

#ifndef AASB_ENGINE_PHONE_CALL_CONTROLLER_AASB_PHONE_CALL_CONTROLLER_ENGINE_SERVICE_H
#define AASB_ENGINE_PHONE_CALL_CONTROLLER_AASB_PHONE_CALL_CONTROLLER_ENGINE_SERVICE_H

#include <AACE/Engine/AASB/AASBHandlerEngineService.h>
#include <AACE/Engine/AASB/AASBEngineService.h>

namespace aasb {
namespace engine {
namespace phoneCallController {

class AASBPhoneCallControllerEngineService : public aace::engine::aasb::AASBHandlerEngineService {
public:
    DESCRIBE("aasb.phoneCallController", VERSION("1.0"), DEPENDS(aace::engine::aasb::AASBEngineService))

private:
    AASBPhoneCallControllerEngineService(const aace::engine::core::ServiceDescription& description);

protected:
    bool postRegister() override;

public:
    virtual ~AASBPhoneCallControllerEngineService() = default;
};

}  // namespace phoneCallController
}  // namespace engine
}  // namespace aasb

#endif
