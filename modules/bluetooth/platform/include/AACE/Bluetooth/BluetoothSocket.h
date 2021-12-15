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

#ifndef AACE_BLUETOOTH_BLUETOOTH_SOCKET_H
#define AACE_BLUETOOTH_BLUETOOTH_SOCKET_H

#include <cstdint>
#include <cstddef>

namespace aace {
namespace bluetooth {

class BluetoothSocket {
protected:
    BluetoothSocket() = default;

public:
    virtual ~BluetoothSocket() = default;

    /**
     * Reads up to `len` bytes of data from the socket into an array of bytes.
     *
     * @param data the buffer into which the data is read.
     * @param off the start offset at which the data is written.
     * @param len the maximum number of bytes to read.
     * @return the total number of bytes read into the buffer, or -1 if there is no more data because the end of the stream has been reached.
     */
    virtual int read(uint8_t* data, size_t off, size_t len) = 0;

    /**
     * Writes `len` bytes from the specified byte array starting at offset `off` to the socket.
     *
     * @param data the buffer containing the data.
     * @param off the start offset at which the data is read
     * @param len the number of bytes to write
     */
    virtual void write(uint8_t* data, size_t off, size_t len) = 0;

    /**
     * Closes the socket and aborts all ongoing operations.
     */
    virtual void close() = 0;
};

}  // namespace bluetooth
}  // namespace aace

#endif
