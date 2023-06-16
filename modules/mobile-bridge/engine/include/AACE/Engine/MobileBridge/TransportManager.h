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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_TRANSPORT_MANAGER_H
#define AACE_ENGINE_MOBILE_BRIDGE_TRANSPORT_MANAGER_H

#include <memory>
#include <string>

#include "AACE/Engine/MobileBridge/TcpProxy.h"
#include "AACE/Engine/MobileBridge/TransportLoop.h"
#include "AACE/Engine/MobileBridge/UdpProxy.h"
#include "AACE/Engine/Utils/Threading/Executor.h"
#include "AACE/MobileBridge/Transport.h"

namespace aace {
namespace engine {
namespace mobileBridge {

class TransportManager : public TransportLoop::Listener {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onActiveTransportChange(const std::string& transportId, const std::string& transportState) = 0;
        virtual void onDeviceHandshaked(
            const std::string& transportId,
            const std::string& deviceToken,
            const std::string& friendlyName) = 0;
        virtual void onInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) = 0;
    };

    TransportManager(
        std::shared_ptr<TcpProxy> tcpProxy,
        std::shared_ptr<UdpProxy> udpProxy,
        std::string deviceTypeId,
        std::shared_ptr<Listener> listener = nullptr);
    ~TransportManager() override;

    void regiserTransport(std::shared_ptr<aace::mobileBridge::Transport> transport);
    void sendTcpData(uint32_t connId, const TcpProxy::DataPiece& piece);
    void sendUdpData(int datagramId, const UdpProxy::Datagram& datagram);
    void authorizeDevice(const std::string& deviceToken, bool authorized);
    void sendInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info);

    // TransportLoop::Listener
    void onTransportState(const std::string& transportId, TransportLoop::State state) override;
    void onOutputStreamReady(const std::string& transportId, std::shared_ptr<DataOutputStream> stream) override;
    TransportLoop::Handling onIncomingData(const std::string& transportId, std::shared_ptr<DataInputStream> stream)
        override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_TRANSPORT_MANAGER_H
