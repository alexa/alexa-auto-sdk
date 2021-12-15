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

#ifndef AACE_BLUETOOTH_GATT_SERVER_H
#define AACE_BLUETOOTH_GATT_SERVER_H

#include "BluetoothEngineInterfaces.h"

namespace aace {
namespace bluetooth {

class GATTServer {
protected:
    GATTServer() = default;

public:
    virtual ~GATTServer();

    using ConnectionState = aace::bluetooth::GATTServerEngineInterface::ConnectionState;

    virtual bool start(const std::string& configuration) = 0;
    virtual bool stop() = 0;
    virtual bool setCharacteristicValue(
        const std::string& serviceId,
        const std::string& characteristicId,
        ByteArrayPtr data) = 0;

    void connectionStateChanged(const std::string& device, ConnectionState state);
    bool requestCharacteristic(
        const std::string& device,
        int requestId,
        const std::string& serviceId,
        const std::string& characteristicId,
        ByteArrayPtr data);
    bool requestDescriptor(
        const std::string& device,
        int requestId,
        const std::string& serviceId,
        const std::string& characteristicId,
        const std::string& descriptorId,
        ByteArrayPtr data);

    /**
     * @internal
     * Sets the Engine interface delegate
     * 
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface(const std::shared_ptr<GATTServerEngineInterface>& gattServerEngineInterface);

private:
    std::weak_ptr<GATTServerEngineInterface> m_gattServerEngineInterface;
};

}  // namespace bluetooth
}  // namespace aace

#endif
