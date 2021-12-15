/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <iostream>
#include <utility>

#include <AACE/Engine/Bluetooth/BluetoothEngineService.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace bluetooth {

// String to identify log entries originating from this file.
static const char* TAG("aace.bluetooth.BluetoothEngineService");

// register the service
REGISTER_SERVICE(BluetoothEngineService);

BluetoothEngineService::BluetoothEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool BluetoothEngineService::initialize() {
    try {
        ThrowIfNot(
            registerServiceInterface<BluetoothServiceInterface>(shared_from_this()),
            "registerBluetoothServiceInterfaceFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool BluetoothEngineService::initializeGATTServer() {
    try {
        // attempt to initialize the gatt server if it hasn't already been initialized,
        // and the application has registered a bluetooth provider
        if (m_gattServerEngineImpl == nullptr && m_bluetoothProvider != nullptr) {
            auto gattServer = m_bluetoothProvider->createGATTServer();
            ThrowIfNull(gattServer, "createGATTServerFailed");

            // register the gatt server platform interface
            ThrowIfNot(registerPlatformInterfaceType(gattServer), "registerGATTServerFailed");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool BluetoothEngineService::start() {
    try {
        if (m_gattServerEngineImpl != nullptr) {
            m_gattServerEngineImpl->start();
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool BluetoothEngineService::stop() {
    try {
        if (m_gattServerEngineImpl != nullptr) {
            m_gattServerEngineImpl->stop();
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool BluetoothEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    try {
        ReturnIf(registerPlatformInterfaceType<aace::bluetooth::BluetoothProvider>(platformInterface), true);
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool BluetoothEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::bluetooth::BluetoothProvider> bluetoothProvider) {
    try {
        ThrowIfNotNull(m_bluetoothProvider, "platformInterfaceAlreadyRegistered");

        // create the gatt server engine impl
        m_bluetoothProvider = std::move(bluetoothProvider);

        // if there are gatt services registered then attempt to create a new gatt server instance
        // and register the services with the server...
        if (!m_gattServiceList.empty()) {
            ThrowIfNot(initializeGATTServer(), "initializeGATTServerFailed");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("interface", "aace::bluetooth::GATTServer").d("reason", ex.what()));
        return false;
    }
}

bool BluetoothEngineService::registerPlatformInterfaceType(
    const std::shared_ptr<aace::bluetooth::GATTServer>& gattServer) {
    try {
        ThrowIfNotNull(m_gattServerEngineImpl, "platformInterfaceAlreadyRegistered");

        // create the gatt server engine impl
        m_gattServerEngineImpl = GATTServerEngineImpl::create(gattServer);
        ThrowIfNull(m_gattServerEngineImpl, "createGATTServerEngineImplFailed");

        // iterate through the gatt service list and add the services
        // to the new gatt server engine impl...
        for (auto& it : m_gattServiceList) {
            if (auto m_gattServiceList_lock = it.lock()) {
                m_gattServerEngineImpl->addService(m_gattServiceList_lock);
            } else {
                AACE_ERROR(LX(TAG).d("reason", "invalidServiceReference"));
            }
        }

        // clear the gatt service list
        m_gattServiceList.clear();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("interface", "aace::bluetooth::GATTServer").d("reason", ex.what()));
        return false;
    }
}

//
// BluetoothServiceInterface
//

bool BluetoothEngineService::addGATTService(std::shared_ptr<aace::engine::bluetooth::GATTService> service) {
    try {
        ThrowIfNull(service, "invalidService");

        // attempt to initialize the gatt server - will succeed if already initialized
        ThrowIfNot(initializeGATTServer(), "initializeGATTServerFailed");

        // TODO: we need to synchronize access to the GATT service list if there is any module
        // calling this method after engine initialization.

        // add the gatt service to the server or save it in the service list
        // if the gatt server has not been initialized yet
        if (m_gattServerEngineImpl != nullptr) {
            m_gattServerEngineImpl->addService(service);
        } else {
            m_gattServiceList.push_back(service);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("interface", "aace::bluetooth::GATTServer").d("reason", ex.what()));
        return false;
    }
}

bool BluetoothEngineService::removeGATTService(aace::engine::bluetooth::GATTService* service) {
    try {
        // TODO: we need to synchronize access to the GATT service list if there is any module
        // calling this method after engine initialization.

        ThrowIfNull(service, "invalidService");
        for (auto it = m_gattServiceList.begin(); it != m_gattServiceList.end(); ++it) {
            if (!it->expired() && it->lock().get() == service) {
                m_gattServiceList.erase(it);
                break;
            }
        }
        if (m_gattServerEngineImpl) {
            return m_gattServerEngineImpl->removeService(service);
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("interface", "aace::bluetooth::GATTServer").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<aace::bluetooth::BluetoothServerSocket> BluetoothEngineService::listenUsingRfcomm(
    const std::string& name,
    const std::string& uuid) {
    return m_bluetoothProvider ? m_bluetoothProvider->listenUsingRfcomm(name, uuid) : nullptr;
}

std::shared_ptr<aace::bluetooth::BluetoothServerSocket> BluetoothEngineService::listenUsingiAP2(
    const std::string& protocol) {
    return m_bluetoothProvider ? m_bluetoothProvider->listenUsingiAP2(protocol) : nullptr;
}

}  // namespace bluetooth
}  // namespace engine
}  // namespace aace
