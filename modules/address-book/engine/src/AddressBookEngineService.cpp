/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <rapidjson/error/en.h>
#include <rapidjson/pointer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Alexa/AlexaEngineService.h>
#include <AACE/Engine/Network/NetworkEngineService.h>

#include <AACE/Engine/AddressBook/AddressBookEngineService.h>

namespace aace {
namespace engine {
namespace addressBook {

// String to identify log entries originating from this file.
static const std::string TAG("aace.addressBook.addressBookEngineService");

// register the service
REGISTER_SERVICE(AddressBookEngineService);

AddressBookEngineService::AddressBookEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

AddressBookEngineService::~AddressBookEngineService() = default;

bool AddressBookEngineService::shutdown() {
    if (m_addressBookCloudUploader != nullptr) {
        m_addressBookCloudUploader->shutdown();
        m_addressBookCloudUploader.reset();
    }
    if (m_addressBookEngineImpl != nullptr) {
        m_addressBookEngineImpl->shutdown();
        m_addressBookEngineImpl.reset();
    }

    return true;
}

bool AddressBookEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::addressBook::AddressBook>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool AddressBookEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::addressBook::AddressBook> platformInterface) {
    try {
        AACE_INFO(LX(TAG, "registerPlatformInterfaceType"));
        ThrowIfNotNull(m_addressBookEngineImpl, "platformInterfaceAlreadyRegistered");

        auto alexaComponents =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponents, "invalidAlexaComponentInterface");

        m_addressBookEngineImpl = AddressBookEngineImpl::create(platformInterface);
        ThrowIfNull(m_addressBookEngineImpl, "createAddressBookEngineImplFailed");

        ThrowIfNot(
            registerServiceInterface<AddressBookServiceInterface>(m_addressBookEngineImpl),
            "registerAddressBookServiceInterfaceFailed");

        auto alexaComponentInterface =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponentInterface, "alexaComponentInterfaceInvalid");

        auto authDelegate = alexaComponentInterface->getAuthDelegate();
        ThrowIfNull(authDelegate, "authDeleteInterfaceInvalid");

        auto deviceInfo = alexaComponentInterface->getDeviceInfo();
        ThrowIfNull(deviceInfo, "deviceInfoInvalid");

        auto networkProvider = getContext()->getServiceInterface<aace::network::NetworkInfoProvider>("aace.network");

        // get the initial network status from the network provider - if the network provider is not
        // available then we always treat the network status as CONNECTED
        auto networkStatus = networkProvider != nullptr ? networkProvider->getNetworkStatus()
                                                        : aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED;

        auto networkObserver =
            getContext()->getServiceInterface<aace::engine::network::NetworkObservableInterface>("aace.network");

        auto alexaEndpoints =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaEndpointInterface>("aace.alexa");
        ThrowIfNull(alexaEndpoints, "alexaEndpointsInvalid");

        m_addressBookCloudUploader = aace::engine::addressBook::AddressBookCloudUploader::create(
            m_addressBookEngineImpl, authDelegate, deviceInfo, networkStatus, networkObserver, alexaEndpoints);
        ThrowIfNull(m_addressBookCloudUploader, "createAddressBookCloudUploaderFailed");

        // set the engine interface reference
        platformInterface->setEngineInterface(m_addressBookEngineImpl);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "registerPlatformInterfaceType<aace::addressBook::AddressBook>").d("reason", ex.what()));
        return false;
    }
}

}  // namespace addressBook
}  // namespace engine
}  // namespace aace
