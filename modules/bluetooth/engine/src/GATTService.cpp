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

#include <AACE/Engine/Bluetooth/GATTService.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <utility>

namespace aace {
namespace engine {
namespace bluetooth {

// String to identify log entries originating from this file.
static const char* TAG("aace.bluetooth.GATTService");

GATTService::GATTService(std::string id) : m_id(std::move(id)) {
}

GATTService::~GATTService() = default;

std::string GATTService::getId() {
    return m_id;
}

bool GATTService::setCharacteristicValue(const std::string& characteristicId, aace::bluetooth::ByteArrayPtr data) {
    try {
        if (auto serverInterface_lock = m_serverInterface.lock()) {
            ThrowIfNot(
                serverInterface_lock->setCharacteristicValue(getId(), characteristicId, std::move(data)),
                "notifyServerFailed");
            return true;
        } else {
            Throw("invalidServerInterfaceReference");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void GATTService::connected(const std::string& device) {
}

void GATTService::disconnected(const std::string& device) {
}

bool GATTService::requestCharacteristic(
    const std::string& device,
    int requestId,
    const std::string& characteristicId,
    aace::bluetooth::ByteArrayPtr data) {
    return false;
}

bool GATTService::requestDescriptor(
    const std::string& device,
    int requestId,
    const std::string& characteristicId,
    const std::string& descriptorId,
    aace::bluetooth::ByteArrayPtr data) {
    return false;
}

void GATTService::setServerInterface(const std::shared_ptr<GATTServerInterface>& serverInterface) {
    m_serverInterface = serverInterface;
}

}  // namespace bluetooth
}  // namespace engine
}  // namespace aace
