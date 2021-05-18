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

#ifndef AACE_BLUETOOTH_PROVIDER_H
#define AACE_BLUETOOTH_PROVIDER_H

#include <AACE/Core/PlatformInterface.h>

#include "GATTServer.h"
#include "BluetoothServerSocket.h"

namespace aace {
namespace bluetooth {

/**
 * Provide RFCOMM and/or GATT implementation.
 */
class BluetoothProvider : public aace::core::PlatformInterface {
protected:
    BluetoothProvider() = default;

public:
    ~BluetoothProvider() override;

    /**
     * Create a GATT Server.
     *
     * @return the created GATT server. nullptr if GATT is not supported.
     */
    virtual std::shared_ptr<GATTServer> createGATTServer();

    /**
     * Create a listening and secure RFCOMM server socket and register the corresponding SDP record. The connection
     * accepted from the server socket should be encrypted and authenticated from trusted device.
     *
     * @param name service name for SDP record
     * @param uuid uuid for SDP record
     * @return the created server socket
     */
    virtual std::shared_ptr<BluetoothServerSocket> listenUsingRfcomm(const std::string& name, const std::string& uuid);

    /**
     * Create a iAP2 server socket with specified protocol.
     *
     * @param protocol the protocol to use when communicating with the device
     * @return the created server socket. nullptr if any error occurs.
     */
    virtual std::shared_ptr<BluetoothServerSocket> listenUsingiAP2(const std::string& protocol);
};

}  // namespace bluetooth
}  // namespace aace

#endif
