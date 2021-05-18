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

#ifndef AACE_ENGINE_BLUETOOTH_BLUETOOTH_SERVICE_INTERFACE_H
#define AACE_ENGINE_BLUETOOTH_BLUETOOTH_SERVICE_INTERFACE_H

#include "GATTService.h"
#include "AACE/Bluetooth/BluetoothServerSocket.h"

namespace aace {
namespace engine {
namespace bluetooth {

class BluetoothServiceInterface {
public:
    virtual ~BluetoothServiceInterface() = default;

    virtual bool addGATTService(std::shared_ptr<aace::engine::bluetooth::GATTService> service) = 0;
    virtual bool removeGATTService(aace::engine::bluetooth::GATTService* service) = 0;
    virtual std::shared_ptr<aace::bluetooth::BluetoothServerSocket> listenUsingRfcomm(
        const std::string& name,
        const std::string& uuid) = 0;
    virtual std::shared_ptr<aace::bluetooth::BluetoothServerSocket> listenUsingiAP2(const std::string& protocol) = 0;
};

}  // namespace bluetooth
}  // namespace engine
}  // namespace aace

#endif
