/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_BLUETOOTH_BLUETOOTH_SERVER_SOCKET_H
#define AACE_BLUETOOTH_BLUETOOTH_SERVER_SOCKET_H

#include <memory>

#include "BluetoothSocket.h"

namespace aace {
namespace bluetooth {

class BluetoothServerSocket {
protected:
    BluetoothServerSocket() = default;

public:
    virtual ~BluetoothServerSocket() = default;

    /**
     * Block until a connection is established. close() can be called from another thread to abort the call.
     *
     * @return a connected BluetoothSocket on successful connection.
     */
    virtual std::shared_ptr<BluetoothSocket> accept() = 0;

    /**
     * Closes the socket and aborts all ongoing operations.
     */
    virtual void close() = 0;
};

}  // namespace bluetooth
}  // namespace aace

#endif