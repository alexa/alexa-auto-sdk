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

#ifndef AACE_JNI_BLUETOOTH_GATT_SERVER_BINDER_H
#define AACE_JNI_BLUETOOTH_GATT_SERVER_BINDER_H

#include <AACE/Bluetooth/GATTServer.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace bluetooth {

class GATTServerHandler : public aace::bluetooth::GATTServer {
public:
    GATTServerHandler(jobject obj);

    // aace::bluetooth::GATTServer
    bool start(const std::string& configuration) override;
    bool stop() override;
    bool setCharacteristicValue(
        const std::string& serviceId,
        const std::string& characteristicId,
        aace::bluetooth::ByteArrayPtr data) override;

private:
    JObject m_obj;
};

class GATTServerBinder {
public:
    GATTServerBinder(jobject obj);

    std::shared_ptr<GATTServerHandler> getGATTServerHandler() {
        return m_gattServerHandler;
    }

private:
    std::shared_ptr<GATTServerHandler> m_gattServerHandler;
};

//
// JConnectionState
//

class JConnectionStateConfig : public EnumConfiguration<GATTServerHandler::ConnectionState> {
public:
    using T = GATTServerHandler::ConnectionState;

    const char* getClassName() override {
        return "com/amazon/aace/bluetooth/GATTServer$ConnectionState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::CONNECTED, "CONNECTED"}, {T::DISCONNECTED, "DISCONNECTED"}};
    }
};

using JConnectionState = JEnum<GATTServerHandler::ConnectionState, JConnectionStateConfig>;

}  // namespace bluetooth
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_BLUETOOTH_GATT_SERVER_BINDER_H
