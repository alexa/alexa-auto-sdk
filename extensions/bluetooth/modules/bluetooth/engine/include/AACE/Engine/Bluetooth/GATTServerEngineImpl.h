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

#ifndef AACE_ENGINE_BLUETOOTH_GATT_SERVER_ENGINE_IMPL_H
#define AACE_ENGINE_BLUETOOTH_GATT_SERVER_ENGINE_IMPL_H

#include <string>
#include <memory>
#include <vector>

#include <AACE/Bluetooth/GATTServer.h>
#include <AACE/Bluetooth/BluetoothEngineInterfaces.h>

#include "GATTService.h"
#include "GATTServerInterface.h"

namespace aace {
namespace engine {
namespace bluetooth {

class GATTServerEngineImpl
        : public GATTServerInterface
        , public aace::bluetooth::GATTServerEngineInterface
        , public std::enable_shared_from_this<GATTServerEngineImpl> {
private:
    explicit GATTServerEngineImpl(std::shared_ptr<aace::bluetooth::GATTServer> gattServerPlatformInterface);

    std::string createServiceConfiguration();

public:
    static std::shared_ptr<GATTServerEngineImpl> create(
        const std::shared_ptr<aace::bluetooth::GATTServer>& gattServerPlatformInterface);

    bool start();
    bool stop();
    bool restart();
    bool addService(const std::shared_ptr<aace::engine::bluetooth::GATTService>& service);
    bool removeService(aace::engine::bluetooth::GATTService* service);

    // GATTServerInterface
    bool setCharacteristicValue(
        const std::string& serviceId,
        const std::string& characteristicId,
        aace::bluetooth::ByteArrayPtr data) override;

    // aace::bluetooth::GATTServerEngineInterface
    void onConnectionStateChanged(const std::string& device, ConnectionState state) override;
    bool onRequestCharacteristic(
        const std::string& device,
        int requestId,
        const std::string& serviceId,
        const std::string& characteristicId,
        aace::bluetooth::ByteArrayPtr data) override;
    bool onRequestDescriptor(
        const std::string& device,
        int requestId,
        const std::string& serviceId,
        const std::string& characteristicId,
        const std::string& descriptorId,
        aace::bluetooth::ByteArrayPtr data) override;

private:
    std::shared_ptr<aace::bluetooth::GATTServer> m_gattServerPlatformInterface;
    std::vector<std::weak_ptr<aace::engine::bluetooth::GATTService>> m_serviceList;
};

}  // namespace bluetooth
}  // namespace engine
}  // namespace aace

#endif
