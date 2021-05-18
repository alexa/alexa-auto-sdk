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

#ifndef AACE_ENGINE_BLUETOOTH_BLUETOOTH_ENGINE_SERVICE_H
#define AACE_ENGINE_BLUETOOTH_BLUETOOTH_ENGINE_SERVICE_H

#include <AACE/Engine/Core/EngineService.h>
#include <AACE/Bluetooth/BluetoothProvider.h>

#include "BluetoothServiceInterface.h"
#include "GATTServerEngineImpl.h"
#include "GATTService.h"

#include <vector>
#include <memory>

namespace aace {
namespace engine {
namespace bluetooth {

class BluetoothEngineService
        : public aace::engine::core::EngineService
        , public BluetoothServiceInterface
        , public std::enable_shared_from_this<BluetoothEngineService> {
public:
    DESCRIBE("aace.bluetooth", VERSION("1.0"))

private:
    explicit BluetoothEngineService(const aace::engine::core::ServiceDescription& description);

    bool initializeGATTServer();

public:
    ~BluetoothEngineService() override = default;

    // BluetoothServiceInterface
    bool addGATTService(std::shared_ptr<aace::engine::bluetooth::GATTService> service) override;
    bool removeGATTService(aace::engine::bluetooth::GATTService* service) override;
    std::shared_ptr<aace::bluetooth::BluetoothServerSocket> listenUsingRfcomm(
        const std::string& name,
        const std::string& uuid) override;
    std::shared_ptr<aace::bluetooth::BluetoothServerSocket> listenUsingiAP2(const std::string& protocol) override;

protected:
    bool initialize() override;
    bool start() override;
    bool stop() override;

    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr && registerPlatformInterfaceType(typedPlatformInterface);
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::bluetooth::BluetoothProvider> bluetoothProvider);
    bool registerPlatformInterfaceType(const std::shared_ptr<aace::bluetooth::GATTServer>& gattServer);

private:
    std::shared_ptr<aace::bluetooth::BluetoothProvider> m_bluetoothProvider;

    std::shared_ptr<aace::engine::bluetooth::GATTServerEngineImpl> m_gattServerEngineImpl;

    std::vector<std::weak_ptr<aace::engine::bluetooth::GATTService>> m_gattServiceList;
};

}  // namespace bluetooth
}  // namespace engine
}  // namespace aace

#endif
