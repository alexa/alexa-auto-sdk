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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_MUXER_H
#define AACE_ENGINE_MOBILE_BRIDGE_MUXER_H

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>

#include "AACE/Engine/MobileBridge/DataStream.h"

namespace aace {
namespace engine {
namespace mobileBridge {

class Muxer {
public:
    Muxer();

    enum Flags : uint32_t {
        SYN = 0x01,  // the first frame for a TCP connection
        FIN = 0x02,  // close the connection gracefully
        RST = 0x04,  // some error happened

        TCP = 0x10,  // a frame for some TCP connection
        UDP = 0x20,  // a frame containing an UDP message

        AUTH = 0x100,  // for handshaking
        INFO = 0x200,  // for exchanging device info periodically
        PING = 0x400,  // for measuring round-trip latency
        PONG = 0x800,  // for measuring round-trip latency

        JSON = 0x1000,  // the payload is of JSON format
    };
    static std::string flagsToString(uint32_t flags);

    struct Frame {
        uint32_t id;
        uint32_t flags;
        std::unique_ptr<uint8_t[]> payload;
        uint32_t len;
    };

    static Frame demux(std::shared_ptr<DataInputStream> stream);
    static void muxTo(
        std::shared_ptr<DataOutputStream> stream,
        uint32_t id,
        uint32_t flags,
        const uint8_t* payload = nullptr,
        uint32_t off = 0,
        uint32_t len = 0);

    using Headers = std::unordered_map<std::string, std::string>;
    static Headers parsePayloadHeaders(const Frame& frame);
    static std::string createHeaders(std::initializer_list<std::pair<std::string, std::string>> headers);
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_MUXER_H
