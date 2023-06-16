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

#include "AACE/Engine/MobileBridge/Config.h"

#include "nlohmann/json.hpp"

namespace aace {
namespace engine {
namespace mobileBridge {

static Config defaultConfig = {
    .tcpProxyPort = 9876,
    .udpProxyPort = 9877,
    .allowedHttpDestPorts = {80, 443},
    .allowedUdpDestPorts = {53},
};

// static
const Config& Config::getDefault() {
    return defaultConfig;
}

void from_json(const nlohmann::json& j, Config& c) {
    c.tcpProxyPort = j.value("tcp-proxy-port", defaultConfig.tcpProxyPort);
    c.udpProxyPort = j.value("udp-proxy-port", defaultConfig.udpProxyPort);
    c.allowedHttpDestPorts = j.value("allowed-http-dest-ports", defaultConfig.allowedHttpDestPorts);
    c.allowedUdpDestPorts = j.value("allowed-udp-dest-ports", defaultConfig.allowedUdpDestPorts);
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
