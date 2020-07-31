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

#include "AACE/Engine/Network/NetworkEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/String/StringUtils.h"

#include "AACE/Network/NetworkProperties.h"

namespace aace {
namespace engine {
namespace network {

// String to identify log entries originating from this file.
static const std::string TAG("aace.network.NetworkEngineService");

// register the service
REGISTER_SERVICE(NetworkEngineService);

NetworkEngineService::NetworkEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool NetworkEngineService::initialize() {
    try {
        ThrowIfNot(registerProperties(), "registerPropertiesFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool NetworkEngineService::registerProperties() {
    try {
        // get the property engine service interface from the property manager service
        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

        propertyManager->registerProperty(aace::engine::propertyManager::PropertyDescription(
            aace::network::property::NETWORK_INTERFACE,
            std::bind(
                &NetworkEngineService::setProperty_networkInterface,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4),
            nullptr));
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}
bool NetworkEngineService::registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::network::NetworkInfoProvider>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool NetworkEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::network::NetworkInfoProvider> networkInfoProvider) {
    try {
        ThrowIfNotNull(m_networkInfoProviderEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the engine implementation
        m_networkInfoProviderEngineImpl = NetworkInfoProviderEngineImpl::create();
        ThrowIfNull(m_networkInfoProviderEngineImpl, "createNetworkInfoProviderEngineImplFailed");

        // create the network observable interface
        ThrowIfNot(
            registerServiceInterface<NetworkObservableInterface>(m_networkInfoProviderEngineImpl),
            "registerNetworkObservableInterfaceFailed");

        // save a reference to the newtork info provider
        m_networkInfoProvider = networkInfoProvider;
        registerServiceInterface<aace::network::NetworkInfoProvider>(m_networkInfoProvider);

        // set the network infor provider engine interface reference
        m_networkInfoProvider->setEngineInterface(m_networkInfoProviderEngineImpl);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<NetworkInfoProvider>").d("reason", ex.what()));
        return false;
    }
}

bool NetworkEngineService::setProperty_networkInterface(
    const std::string& value,
    bool& changed,
    bool& async,
    const SetPropertyResultCallback& callbackFunction) {
    try {
        AACE_INFO(LX(TAG).sensitive("value", value));
        ThrowIfNot(m_networkInfoProviderEngineImpl->setNetworkInterface(value), "setNetworkInterfaceFailed");
        changed = true;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("value", value));
        return false;
    }
}

}  // namespace network
}  // namespace engine
}  // namespace aace
