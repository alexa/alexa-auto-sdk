/*
 * Copyright 2017-2018, 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_BLUETOOTH_BLUETOOTH_PROVIDER_BINDER_H
#define AACE_JNI_BLUETOOTH_BLUETOOTH_PROVIDER_BINDER_H

#include <AACE/Bluetooth/BluetoothProvider.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace bluetooth {

class BluetoothProviderHandler : public aace::bluetooth::BluetoothProvider {
public:
    BluetoothProviderHandler(jobject obj);

    // aace::bluetooth::BluetoothProvider
    std::shared_ptr<aace::bluetooth::GATTServer> createGATTServer() override;
    std::shared_ptr<aace::bluetooth::BluetoothServerSocket> listenUsingRfcomm(
        const std::string& name,
        const std::string& uuid) override;
    std::shared_ptr<aace::bluetooth::BluetoothServerSocket> listenUsingiAP2(const std::string& protocol) override;

private:
    JObject m_obj;
};

class BluetoothProviderBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    BluetoothProviderBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_bluetoothProviderHandler;
    }

    std::shared_ptr<BluetoothProviderHandler> getBluetoothProvider() {
        return m_bluetoothProviderHandler;
    }

private:
    std::shared_ptr<BluetoothProviderHandler> m_bluetoothProviderHandler;
};

}  // namespace bluetooth
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_BLUETOOTH_BLUETOOTH_PROVIDER_BINDER_H
