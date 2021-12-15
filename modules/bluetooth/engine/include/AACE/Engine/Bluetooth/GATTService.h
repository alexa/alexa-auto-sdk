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

#ifndef AACE_ENGINE_BLUETOOTH_GATT_SERVICE_H
#define AACE_ENGINE_BLUETOOTH_GATT_SERVICE_H

#include <string>
#include <initializer_list>
#include <memory>
#include <vector>
#include <ostream>

#include "GATTServerInterface.h"

namespace aace {
namespace engine {
namespace bluetooth {

class GATTService {
protected:
    explicit GATTService(std::string id);

public:
    virtual ~GATTService();

    std::string getId();

    bool setCharacteristicValue(const std::string& characteristicId, aace::bluetooth::ByteArrayPtr data);

    virtual std::string getConfiguration() = 0;

    virtual void connected(const std::string& device);
    virtual void disconnected(const std::string& device);
    virtual bool requestCharacteristic(
        const std::string& device,
        int requestId,
        const std::string& characteristicId,
        aace::bluetooth::ByteArrayPtr data);
    virtual bool requestDescriptor(
        const std::string& device,
        int requestId,
        const std::string& characteristicId,
        const std::string& descriptorId,
        aace::bluetooth::ByteArrayPtr data);

    void setServerInterface(const std::shared_ptr<GATTServerInterface>& serverInterface);

private:
    const std::string m_id;

    std::weak_ptr<GATTServerInterface> m_serverInterface;
};

}  // namespace bluetooth
}  // namespace engine
}  // namespace aace

#endif
