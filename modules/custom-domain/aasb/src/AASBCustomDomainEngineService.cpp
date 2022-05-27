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

#include <AACE/Engine/MessageBroker/MessageBrokerInterface.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AASB/Engine/CustomDomain/AASBCustomDomain.h>
#include <AASB/Engine/CustomDomain/AASBCustomDomainEngineService.h>

namespace aasb {
namespace engine {
namespace customDomain {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.customDomain.AASBCustomDomainEngineService");

// Minimum version this module supports
static const aace::engine::core::Version minRequiredVersion = VERSION("4.0");

// register the service
REGISTER_SERVICE(AASBCustomDomainEngineService);

AASBCustomDomainEngineService::AASBCustomDomainEngineService(
    const aace::engine::core::ServiceDescription& description) :
        aace::engine::messageBroker::MessageHandlerEngineService(
            description,
            minRequiredVersion,
            {},
            {"CustomDomain"}) {
}

bool AASBCustomDomainEngineService::postRegister() {
    try {
        auto messageBrokerServiceInterface =
            getContext()->getServiceInterface<aace::engine::messageBroker::MessageBrokerServiceInterface>(
                "aace.messageBroker");
        ThrowIfNull(messageBrokerServiceInterface, "invalidMessageBrokerServiceInterface");

        // CustomDomain
        if (isInterfaceEnabled("CustomDomain")) {
            auto customDomain = AASBCustomDomain::create(messageBrokerServiceInterface->getMessageBroker());
            ThrowIfNull(customDomain, "invalidCustomDomainHandler");
            getContext()->registerPlatformInterface(customDomain);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace customDomain
}  // namespace engine
}  // namespace aasb
