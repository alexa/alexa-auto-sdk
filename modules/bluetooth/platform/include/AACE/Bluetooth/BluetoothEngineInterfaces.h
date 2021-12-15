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

#ifndef AACE_BLUETOOTH_BLUETOOTH_ENGINE_INTERFACES_H
#define AACE_BLUETOOTH_BLUETOOTH_ENGINE_INTERFACES_H

#include <string>
#include <memory>
#include <vector>

#include "ByteArray.h"

namespace aace {
namespace bluetooth {

class GATTServerEngineInterface {
public:
    virtual ~GATTServerEngineInterface() = default;

    enum class ConnectionState { CONNECTED, DISCONNECTED };

    virtual void onConnectionStateChanged(const std::string& device, ConnectionState state) = 0;
    virtual bool onRequestCharacteristic(
        const std::string& device,
        int requestId,
        const std::string& serviceId,
        const std::string& characteristicId,
        ByteArrayPtr data) = 0;
    virtual bool onRequestDescriptor(
        const std::string& device,
        int requestId,
        const std::string& serviceId,
        const std::string& characteristicId,
        const std::string& descriptorId,
        ByteArrayPtr data) = 0;
};

}  // namespace bluetooth
}  // namespace aace

#endif
