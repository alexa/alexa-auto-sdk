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

#ifndef AACE_ENGINE_BLUETOOTH_GATT_DESCRIPTOR_H
#define AACE_ENGINE_BLUETOOTH_GATT_DESCRIPTOR_H

#include <string>

namespace aace {
namespace engine {
namespace bluetooth {

static const uint8_t PERMISSON_READ = 1 << 0;
static const uint8_t PERMISSON_READ_ENCRYPTED = 1 << 1;
static const uint8_t PERMISSON_READ_ENCRYPTED_MITM = 1 << 2;
static const uint8_t PERMISSON_WRITE = 1 << 3;
static const uint8_t PERMISSON_WRITE_ENCRYPTED = 1 << 4;
static const uint8_t PERMISSON_WRITE_ENCRYPTED_MITM = 1 << 5;
static const uint8_t PERMISSON_WRITE_SIGNED = 1 << 6;
static const uint8_t PERMISSON_WRITE_SIGNED_MITM = 1 << 6;

class GATTDescriptor {
public:
    using Permissions = uint8_t;

    GATTDescriptor(std::string id, Permissions permissions);

    std::string getId();

    bool hasPermissions(Permissions permissions);

private:
    const std::string m_id;
    const Permissions m_permissions;
};

}  // namespace bluetooth
}  // namespace engine
}  // namespace aace

#endif
