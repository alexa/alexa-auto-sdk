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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_SESSION_MANAGER_H
#define AACE_ENGINE_MOBILE_BRIDGE_SESSION_MANAGER_H

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

class SessionManager {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onProtectSocket(int sock) = 0;
    };

    SessionManager(int tcpProxyPort = -1, int udpProxyPort = -1, std::shared_ptr<Listener> listener = nullptr);
    ~SessionManager();

    void start(int tunFd);
    void stop();

    void shutdown();

    struct Statistics {
        size_t numIpPackets;
        size_t totalUpstreamBytes;
    };
    Statistics getStatistics();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_SESSION_MANAGER_H
