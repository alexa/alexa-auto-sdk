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

#include <typeinfo>
#include <climits>
#include <iostream>

#include <AACE/Engine/AASB/AASBEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/MessageBroker/MessageBrokerServiceInterface.h>
#include <AACE/Engine/Utils/JSON/JSON.h>
#ifdef HAS_AASB_MESSAGES
#include <AASB/Message/AASB/Service/StartServiceMessage.h>
#include <AASB/Message/AASB/Service/StopServiceMessage.h>
#endif

namespace aace {
namespace engine {
namespace aasb {

// String to identify log entries originating from this file.
static const std::string TAG("aace.aasb.AASBEngineService");

// register the service
REGISTER_SERVICE(AASBEngineService);

AASBEngineService::AASBEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool AASBEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        Throw("AASB configuration has been deprecated, use 'aace.messageBroker' to configure the service instead.");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::start() {
    try {
#ifdef HAS_AASB_MESSAGES
        auto messageBrokerServiceInterface =
            getContext()->getServiceInterface<aace::engine::messageBroker::MessageBrokerServiceInterface>(
                "aace.messageBroker");
        ThrowIfNull(messageBrokerServiceInterface, "invalidMessageBrokerServiceInterface");

        ::aasb::message::aasb::service::StartServiceMessage message;
        messageBrokerServiceInterface->getMessageBroker()->publish(message.toString()).send();
#endif
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::stop() {
    try {
#ifdef HAS_AASB_MESSAGES
        auto messageBrokerServiceInterface =
            getContext()->getServiceInterface<aace::engine::messageBroker::MessageBrokerServiceInterface>(
                "aace.messageBroker");
        ThrowIfNull(messageBrokerServiceInterface, "invalidMessageBrokerServiceInterface");

        ::aasb::message::aasb::service::StopServiceMessage message;
        messageBrokerServiceInterface->getMessageBroker()->publish(message.toString()).send();
#endif
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::aasb::AASB>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBEngineService::registerPlatformInterfaceType(std::shared_ptr<aace::aasb::AASB> aasb) {
    try {
        ThrowIfNotNull(m_aasbEngineImpl, "platformInterfaceAlreadyRegistered");

        auto messageBrokerServiceInterface =
            getContext()->getServiceInterface<aace::engine::messageBroker::MessageBrokerServiceInterface>(
                "aace.messageBroker");
        ThrowIfNull(messageBrokerServiceInterface, "invalidMessageBrokerServiceInterface");

        m_aasbEngineImpl = AASBEngineImpl::create(
            aasb, messageBrokerServiceInterface->getMessageBroker(), messageBrokerServiceInterface->getStreamManager());
        ThrowIfNull(m_aasbEngineImpl, "createAASBEngineImplFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<AASB>").d("reason", ex.what()));
        return false;
    }
}

}  // namespace aasb
}  // namespace engine
}  // namespace aace
