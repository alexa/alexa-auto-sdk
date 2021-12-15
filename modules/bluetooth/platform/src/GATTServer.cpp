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

#include <AACE/Bluetooth/GATTServer.h>

namespace aace {
namespace bluetooth {

GATTServer::~GATTServer() = default;

void GATTServer::setEngineInterface(const std::shared_ptr<GATTServerEngineInterface>& gattServerEngineInterface) {
    m_gattServerEngineInterface = gattServerEngineInterface;
}

void GATTServer::connectionStateChanged(const std::string& device, ConnectionState state) {
    if (auto m_gattServerEngineInterface_lock = m_gattServerEngineInterface.lock()) {
        m_gattServerEngineInterface_lock->onConnectionStateChanged(device, state);
    }
}

bool GATTServer::requestCharacteristic(
    const std::string& device,
    int requestId,
    const std::string& serviceId,
    const std::string& characteristicId,
    ByteArrayPtr data) {
    auto m_gattServerEngineInterface_lock = m_gattServerEngineInterface.lock();
    return m_gattServerEngineInterface_lock != nullptr &&
           m_gattServerEngineInterface_lock->onRequestCharacteristic(
               device, requestId, serviceId, characteristicId, std::move(data));
}

bool GATTServer::requestDescriptor(
    const std::string& device,
    int requestId,
    const std::string& serviceId,
    const std::string& characteristicId,
    const std::string& descriptorId,
    ByteArrayPtr data) {
    auto m_gattServerEngineInterface_lock = m_gattServerEngineInterface.lock();
    return m_gattServerEngineInterface_lock != nullptr &&
           m_gattServerEngineInterface_lock->onRequestDescriptor(
               device, requestId, serviceId, characteristicId, descriptorId, std::move(data));
}

}  // namespace bluetooth
}  // namespace aace
