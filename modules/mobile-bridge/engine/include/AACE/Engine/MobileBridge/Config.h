/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_MOBILE_CONFIG_H
#define AACE_ENGINE_MOBILE_BRIDGE_MOBILE_CONFIG_H

#include <vector>

#include "nlohmann/json_fwd.hpp"

namespace aace {
namespace engine {
namespace mobileBridge {

struct Config {
    int tcpProxyPort;                       // TCP port to bind for local HTTP proxy
    int udpProxyPort;                       // UDP port to bind for UDP proxy
    std::vector<int> allowedHttpDestPorts;  // the list of destination TCP ports allowed
    std::vector<int> allowedUdpDestPorts;   // the list of destination UDP ports allowed

    static const Config& getDefault();
};

void from_json(const nlohmann::json& j, Config& c);

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_MOBILE_CONFIG_H
