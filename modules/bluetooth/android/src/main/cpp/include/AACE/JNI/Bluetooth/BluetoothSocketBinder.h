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

#ifndef AACE_JNI_BLUETOOTH_BLUETOOTH_SOCKET_H
#define AACE_JNI_BLUETOOTH_BLUETOOTH_SOCKET_H

#include <AACE/Bluetooth/BluetoothSocket.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace bluetooth {

class BluetoothSocketBinder : public aace::bluetooth::BluetoothSocket {
public:
    BluetoothSocketBinder(jobject obj);

    // aace::bluetooth::BluetoothSocket
    int read(uint8_t* data, size_t off, size_t len) override;
    void write(uint8_t* data, size_t off, size_t len) override;
    void close() override;

private:
    JObject m_obj;
};

}  // namespace bluetooth
}  // namespace jni
}  // namespace aace

#endif
