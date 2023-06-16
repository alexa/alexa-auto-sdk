/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AASB_ENGINE_MOBILE_BRIDGE_AASB_MOBILE_BRIDGE_ENGINE_SERVICE_H
#define AASB_ENGINE_MOBILE_BRIDGE_AASB_MOBILE_BRIDGE_ENGINE_SERVICE_H

#include <mutex>
#include <unordered_map>

#include "AACE/Engine/MessageBroker/MessageBrokerEngineService.h"
#include "AACE/Engine/MessageBroker/MessageHandlerEngineService.h"
#include "AACE/Engine/MobileBridge/MobileBridgeEngineService.h"

namespace aasb {
namespace engine {
namespace mobileBridge {

class AASBMobileBridgeEngineService : public aace::engine::messageBroker::MessageHandlerEngineService {
public:
    DESCRIBE(
        "aasb.mobileBridge",
        VERSION("1.0"),
        DEPENDS(aace::engine::messageBroker::MessageBrokerEngineService),
        DEPENDS(aace::engine::mobileBridge::MobileBridgeEngineService))

private:
    AASBMobileBridgeEngineService(const aace::engine::core::ServiceDescription& description);

protected:
    bool postRegister() override;

public:
    virtual ~AASBMobileBridgeEngineService() = default;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aasb

#endif
