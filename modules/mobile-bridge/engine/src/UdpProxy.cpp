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

#include "AACE/Engine/MobileBridge/UdpProxy.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/MobileBridge/Util.h"

namespace aace {
namespace engine {
namespace mobileBridge {

struct UdpProxy::Impl {
    static constexpr const char* TAG = "UdpProxy::Impl";

    static constexpr int INVALID_FD = -1;
    static constexpr int DATAGRAM_BUFFER_BYTES = 1024;

    std::thread m_serverThread;
    int m_serverSocket = INVALID_FD;
    uint32_t m_datagramId = 0;  // counter for UDP datagram identifier

    struct ReturnAddress {
        struct sockaddr addr;
        socklen_t addr_len;
        std::chrono::time_point<std::chrono::steady_clock> createdTime;

        ReturnAddress() {
            memset(&addr, 0, sizeof(addr));
            addr_len = 0;
            createdTime = std::chrono::steady_clock::now();
        }
    };
    std::mutex m_returnAddressMutex;
    std::unordered_map<uint32_t, ReturnAddress> m_returnAddresses;

    Impl(int port, DatagramHandler handler) {
        m_serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (m_serverSocket < 0) {
            AACE_ERROR(LX(TAG).m("Failed to create server socket").d("serverSocket", m_serverSocket));
            throw std::runtime_error("Failed to create server socket");
        }
        const int reuse = 1;
        if (::setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
            AACE_ERROR(LX(TAG).m("Failed to set SO_REUSEADDR").e("errno", errno));
        }
        if (::setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(int)) < 0) {
            AACE_ERROR(LX(TAG).m("Failed to set SO_REUSEADDR").e("errno", errno));
        }

        m_serverThread = std::thread([this, port, handler] {
            setThreadName("UdpProxy");
            serverLoop(port, handler);
        });
    }

    void serverLoop(int port, DatagramHandler handler) {
        AACE_DEBUG(LX(TAG).d("port", port));

        int ret;
        uint8_t buffer[DATAGRAM_BUFFER_BYTES];

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        auto* serv_sockaddr = reinterpret_cast<struct sockaddr*>(&serv_addr);

        ret = ::bind(m_serverSocket, serv_sockaddr, sizeof(serv_addr));
        if (ret < 0) {
            AACE_ERROR(LX(TAG).m("Failed to bind socket").d("error", ret));
            return;
        }

        while (true) {
            struct sockaddr_in src_addr;
            socklen_t addr_len = sizeof(src_addr);
            auto len = ::recvfrom(
                m_serverSocket,
                reinterpret_cast<char*>(buffer),
                sizeof(buffer),
                MSG_WAITALL,
                reinterpret_cast<struct sockaddr*>(&src_addr),
                &addr_len);
            if (len < 0) {
                AACE_ERROR(LX(TAG).m("Failed to receive packet").d("error", ret));
                break;
            }
            AACE_DEBUG(LX(TAG).m("Received message").d("len", len));

            Datagram datagram;
            datagram.buf = buffer;
            datagram.off = 0;
            datagram.len = static_cast<int>(len);
            handler(++m_datagramId, datagram);

            addReturnAddress(m_datagramId, reinterpret_cast<struct sockaddr*>(&src_addr), addr_len);
        }
    }

    void addReturnAddress(uint32_t datagramId, struct sockaddr* addr, socklen_t addr_len) {
        ReturnAddress ra;
        memcpy(&ra.addr, addr, addr_len);
        ra.addr_len = addr_len;

        std::lock_guard<std::mutex> lock(m_returnAddressMutex);
        m_returnAddresses.emplace(m_datagramId, ra);
    }

    void sendReply(uint32_t datagramId, uint8_t* buf, int off, uint32_t len) {
        std::unique_lock<std::mutex> lock(m_returnAddressMutex);

        auto it = m_returnAddresses.find(datagramId);
        if (it != m_returnAddresses.end()) {
            auto ra = it->second;

            lock.unlock();
            ::sendto(m_serverSocket, buf + off, len, 0, &ra.addr, ra.addr_len);

            lock.lock();
            m_returnAddresses.erase(datagramId);
        }

        // TODO: clean out-of-dated return addresses
    }

    void shutdown() {
        if (m_serverSocket >= 0) {
            AACE_INFO(LX(TAG).m("Closing server socket"));
            ::shutdown(m_serverSocket, SHUT_RDWR);
            ::close(m_serverSocket);
            m_serverThread.join();
            m_serverSocket = INVALID_FD;
        }
    }
};

// String to identify log entries originating from this file.
static const char* TAG = "UdpProxy";

UdpProxy::UdpProxy(int port, DatagramHandler handler) {
    AACE_INFO(LX(TAG).d("port", port));
    m_impl = std::make_unique<Impl>(port, handler);
}

UdpProxy::~UdpProxy() {
    shutdown();
}

void UdpProxy::sendReply(uint32_t datagramId, uint8_t* buf, int off, uint32_t len) {
    m_impl->sendReply(datagramId, buf, off, len);
}

void UdpProxy::shutdown() {
    m_impl->shutdown();
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
