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

#include <AASB/Engine/CarControl/AASBCarControlEngineService.h>
#include <AASB/Engine/CarControl/AASBCarControl.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aasb {
namespace engine {
namespace carControl {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.carControl.AASBCarControlEngineService");

// Minimum version this module supports
static const aace::engine::core::Version minRequiredVersion = VERSION("3.0");

// register the service
REGISTER_SERVICE(AASBCarControlEngineService);

AASBCarControlEngineService::AASBCarControlEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::aasb::AASBHandlerEngineService(description, minRequiredVersion, {"CarControl"}) {
}

bool AASBCarControlEngineService::configureAASBInterface(
    const std::string& name,
    bool enabled,
    std::istream& configuration) {
    try {
        // call inherited configure method
        ThrowIfNot(
            AASBHandlerEngineService::configureAASBInterface(name, enabled, configuration),
            "configureAASBInterfaceFailed");

        // handle specific interface configuration options
        if (enabled && name == "CarControl") {
            ThrowIfNot(configureCarControl(configuration), "configureCarControlFailed");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControlEngineService::configureCarControl(std::istream& configuration) {
    try {
        auto root = nlohmann::json::parse(configuration);
        m_asyncReplyTimeout = root["/asyncReplyTimeout"_json_pointer];
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControlEngineService::postRegister() {
    try {
        auto aasbServiceInterface =
            getContext()->getServiceInterface<aace::engine::aasb::AASBServiceInterface>("aace.aasb");
        ThrowIfNull(aasbServiceInterface, "invalidAASBServiceInterface");

        // CarControl
        if (isInterfaceEnabled("CarControl")) {
            auto carControl = AASBCarControl::create(aasbServiceInterface->getMessageBroker(), m_asyncReplyTimeout);
            ThrowIfNull(carControl, "invalidCarControlHandler");
            getContext()->registerPlatformInterface(carControl);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace carControl
}  // namespace engine
}  // namespace aasb
