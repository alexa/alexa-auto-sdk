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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_UDP_PROXY_H
#define AACE_ENGINE_MOBILE_BRIDGE_UDP_PROXY_H

#include <functional>
#include <memory>

namespace aace {
namespace engine {
namespace mobileBridge {

class UdpProxy {
public:
    struct Datagram {
        uint8_t* buf;
        int off;
        int len;
    };

    using DatagramHandler = std::function<void(uint32_t datagramId, const Datagram& datagram)>;

    /**
     * Starts a UDP proxy to listen for datagrams send to the specified port.
     *
     * @param port port number to listen to.
     * @param handler the handle to handle received packets.
     */
    UdpProxy(int port, DatagramHandler handler);
    ~UdpProxy();

    void sendReply(uint32_t datagramId, uint8_t* buf, int off, uint32_t len);

    void shutdown();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_UDP_PROXY_H
