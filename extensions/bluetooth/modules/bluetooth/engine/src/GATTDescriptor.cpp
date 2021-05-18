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

#include <AACE/Engine/Bluetooth/GATTDescriptor.h>

#include <utility>

namespace aace {
namespace engine {
namespace bluetooth {

// String to identify log entries originating from this file.
static const std::string TAG("aace.bluetooth.GATTDescriptor");

GATTDescriptor::GATTDescriptor(std::string id, Permissions permissions) :
        m_id(std::move(id)), m_permissions(permissions) {
}

std::string GATTDescriptor::getId() {
    return m_id;
}

bool GATTDescriptor::hasPermissions(Permissions permissions) {
    return (m_permissions & permissions) == permissions;
}

}  // namespace bluetooth
}  // namespace engine
}  // namespace aace
