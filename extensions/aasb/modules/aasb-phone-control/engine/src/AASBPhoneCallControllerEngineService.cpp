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

#include <AACE/Engine/AASB/MessageBrokerInterface.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AASB/Engine/PhoneCallController/AASBPhoneCallController.h>
#include <AASB/Engine/PhoneCallController/AASBPhoneCallControllerEngineService.h>

namespace aasb {
namespace engine {
namespace phoneCallController {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.phoneCallController.AASBPhoneCallControllerEngineService");

// Minimum version this module supports
static const aace::engine::core::Version minRequiredVersion = VERSION("3.0");

// register the service
REGISTER_SERVICE(AASBPhoneCallControllerEngineService);

AASBPhoneCallControllerEngineService::AASBPhoneCallControllerEngineService(
    const aace::engine::core::ServiceDescription& description) :
        aace::engine::aasb::AASBHandlerEngineService(description, minRequiredVersion, {"PhoneCallController"}) {
}

bool AASBPhoneCallControllerEngineService::postRegister() {
    try {
        auto aasbServiceInterface =
            getContext()->getServiceInterface<aace::engine::aasb::AASBServiceInterface>("aace.aasb");
        ThrowIfNull(aasbServiceInterface, "invalidAASBServiceInterface");

        // PhoneCallController
        if (isInterfaceEnabled("PhoneCallController")) {
            auto pcc = AASBPhoneCallController::create(aasbServiceInterface->getMessageBroker());
            ThrowIfNull(pcc, "invalidPhoneCallControllerHandler");
            getContext()->registerPlatformInterface(pcc);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace phoneCallController
}  // namespace engine
}  // namespace aasb
