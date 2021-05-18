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

#include <AACE/Engine/Bluetooth/GATTServerEngineImpl.h>
#include <AACE/Engine/Utils/UUID/UUID.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <nlohmann/json.hpp>
#include <utility>

namespace aace {
namespace engine {
namespace bluetooth {

// String to identify log entries originating from this file.
static const char* TAG("aace.bluetooth.GATTServerEngineImpl");

GATTServerEngineImpl::GATTServerEngineImpl(std::shared_ptr<aace::bluetooth::GATTServer> gattServerPlatformInterface) :
        m_gattServerPlatformInterface(std::move(gattServerPlatformInterface)) {
}

std::shared_ptr<GATTServerEngineImpl> GATTServerEngineImpl::create(
    const std::shared_ptr<aace::bluetooth::GATTServer>& gattServerPlatformInterface) {
    std::shared_ptr<GATTServerEngineImpl> gattServerEngineImpl = nullptr;

    try {
        ThrowIfNull(gattServerPlatformInterface, "invalidPlatformInterface");
        gattServerEngineImpl =
            std::shared_ptr<GATTServerEngineImpl>(new GATTServerEngineImpl(gattServerPlatformInterface));

        // set the engine interface
        gattServerPlatformInterface->setEngineInterface(gattServerEngineImpl);

        return gattServerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

std::string GATTServerEngineImpl::createServiceConfiguration() {
    try {
        nlohmann::json configuration;

        configuration["services"] = nlohmann::json::array();

        // iterate through the service list and create the configuration json...
        for (auto& it : m_serviceList) {
            if (auto serviceList_lock = it.lock()) {
                nlohmann::json service;

                service["id"] = serviceList_lock->getId();
                service["configuration"] = nlohmann::json::parse(serviceList_lock->getConfiguration());

                configuration["services"].push_back(service);
            } else {
                Throw("invalidServiceReference");
            }
        }

        return configuration.dump(3);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::string();
    }
}

bool GATTServerEngineImpl::start() {
    try {
        ThrowIf(m_serviceList.empty(), "emptyServiceList");

        auto configuration = createServiceConfiguration();
        ThrowIf(configuration.empty(), "invalidServiceConfiguration");

        return m_gattServerPlatformInterface->start(configuration);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool GATTServerEngineImpl::stop() {
    try {
        return m_gattServerPlatformInterface->stop();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool GATTServerEngineImpl::restart() {
    try {
        ThrowIfNot(stop(), "failedToStop");
        if (m_serviceList.empty()) {
            return true;  // do nothing when no service is specified
        }
        return start();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool GATTServerEngineImpl::addService(const std::shared_ptr<aace::engine::bluetooth::GATTService>& service) {
    try {
        ThrowIfNull(service, "invalidService");
        // TODO: we need to synchronize access to the list if there is any module
        // calling this method after engine initialization.
        m_serviceList.push_back(service);

        // set the service's server interface
        service->setServerInterface(shared_from_this());

        return restart();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool GATTServerEngineImpl::removeService(aace::engine::bluetooth::GATTService* service) {
    try {
        ThrowIfNull(service, "invalidService");
        // TODO: we need to synchronize access to the list if there is any module
        // calling this method after engine initialization.
        for (auto it = m_serviceList.begin(); it != m_serviceList.end();) {
            if (it->lock().get() == service) {
                it = m_serviceList.erase(it);
            } else {
                ++it;
            }
        }

        // Restart after removing the service
        return restart();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// GATTServerInterface
//

bool GATTServerEngineImpl::setCharacteristicValue(
    const std::string& serviceId,
    const std::string& characteristicId,
    aace::bluetooth::ByteArrayPtr data) {
    try {
        return m_gattServerPlatformInterface->setCharacteristicValue(serviceId, characteristicId, std::move(data));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::bluetooth::GATTServerEngineInterface
//

void GATTServerEngineImpl::onConnectionStateChanged(const std::string& device, ConnectionState state) {
    // iterate through the service list and update the connection state
    for (auto& it : m_serviceList) {
        if (auto m_service_lock = it.lock()) {
            switch (state) {
                case ConnectionState::CONNECTED:
                    m_service_lock->connected(device);
                    break;
                case ConnectionState::DISCONNECTED:
                    m_service_lock->disconnected(device);
                    break;
            }
        } else {
            AACE_ERROR(LX(TAG).d("reason", "invalidServiceReference"));
        }
    }
}

bool GATTServerEngineImpl::onRequestCharacteristic(
    const std::string& device,
    int requestId,
    const std::string& serviceId,
    const std::string& characteristicId,
    aace::bluetooth::ByteArrayPtr data) {
    try {
        for (auto& it : m_serviceList) {
            if (auto m_service_lock = it.lock()) {
                AACE_DEBUG(LX(TAG).d("service", m_service_lock->getId()));
                if (aace::engine::utils::uuid::compare(m_service_lock->getId(), serviceId)) {
                    return m_service_lock->requestCharacteristic(device, requestId, characteristicId, std::move(data));
                }
            } else {
                AACE_ERROR(LX(TAG).d("reason", "invalidServiceReference"));
            }
        }

        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool GATTServerEngineImpl::onRequestDescriptor(
    const std::string& device,
    int requestId,
    const std::string& serviceId,
    const std::string& characteristicId,
    const std::string& descriptorId,
    aace::bluetooth::ByteArrayPtr data) {
    try {
        for (auto& it : m_serviceList) {
            if (auto m_service_lock = it.lock()) {
                if (aace::engine::utils::uuid::compare(m_service_lock->getId(), serviceId)) {
                    return m_service_lock->requestDescriptor(
                        device, requestId, characteristicId, descriptorId, std::move(data));
                }
            } else {
                AACE_ERROR(LX(TAG).d("reason", "invalidServiceReference"));
            }
        }

        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace bluetooth
}  // namespace engine
}  // namespace aace
