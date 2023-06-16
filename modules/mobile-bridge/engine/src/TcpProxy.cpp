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

#include "AACE/Engine/MobileBridge/TcpProxy.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <stdexcept>
#include <thread>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/MobileBridge/SocketPuller.h"
#include "AACE/Engine/MobileBridge/Util.h"

namespace aace {
namespace engine {
namespace mobileBridge {

struct TcpProxy::Impl {
    static constexpr const char* TAG = "TcpProxy::Impl";

    static constexpr int INVALID_FD = -1;
    static constexpr int LISTEN_BACKLOG = 16;

    std::thread m_serverThread;
    int m_serverSocket = INVALID_FD;
    uint32_t m_connId = 0;  // counter for TCP connection identifier

    struct PullerInfo {
        std::shared_ptr<SocketPuller> puller;
        bool finished;  // has reached EOS or failed.
    };

    std::mutex m_socketsMutex;
    std::unordered_map<uint32_t, PullerInfo> m_socketPullers;

    Impl(int port, DataHandler dataHandler, NewConnectionHandler newConnectionHandler) {
        m_serverSocket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_serverSocket < 0) {
            AACE_ERROR(LX(TAG).m("Failed to create server socket").e("errno", errno));
            throw std::runtime_error("Failed to create server socket");
        }
        const int reuse = 1;
        if (::setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
            AACE_ERROR(LX(TAG).m("Failed to set SO_REUSEADDR").e("errno", errno));
        }
        if (::setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(int)) < 0) {
            AACE_ERROR(LX(TAG).m("Failed to set SO_REUSEADDR").e("errno", errno));
        }

        AACE_INFO(LX(TAG).d("serverSocket", m_serverSocket));
        m_serverThread = std::thread([this, port, dataHandler, newConnectionHandler] {
            setThreadName("TcpProxy");
            serverLoop(port, dataHandler, newConnectionHandler);
        });
    }

    void serverLoop(int port, DataHandler dataHandler, NewConnectionHandler newConnectionHandler) {
        int ret;

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        auto* serv_sockaddr = reinterpret_cast<struct sockaddr*>(&serv_addr);

        ret = ::bind(m_serverSocket, serv_sockaddr, sizeof(serv_addr));
        if (ret < 0) {
            AACE_ERROR(LX(TAG).m("Failed to bind socket").d("sock", m_serverSocket).e("errno", errno));
            return;
        }
        ret = ::listen(m_serverSocket, LISTEN_BACKLOG);
        if (ret < 0) {
            AACE_ERROR(LX(TAG).m("Failed to listen socket").d("sock", m_serverSocket).e("errno", errno));
            return;
        }

        while (true) {
            socklen_t addr_len = sizeof(serv_addr);
            int sock = ::accept(m_serverSocket, serv_sockaddr, &addr_len);
            if (sock < 0) {
                AACE_ERROR(LX(TAG).m("Failed to accept socket").d("sock", m_serverSocket).e("errno", errno));
                break;
            }

            onNewTcpClient(sock, ++m_connId, dataHandler, newConnectionHandler);
            shutdownPullers(false);  // Clean up finished pullers
        }

        shutdownPullers(true);  // force closing all pullers
    }

    // Called in server thread.
    void onNewTcpClient(int sock, uint32_t connId, DataHandler handler, NewConnectionHandler newConnectionHandler) {
        AACE_INFO(LX(TAG).d("sock", sock).d("connId", connId));
        auto puller = std::make_shared<SocketPuller>(
            sock, [this, connId, handler](uint8_t* buf, int off, int len, size_t bytesSoFar) {
                // Run in puller thread
                if (len >= 0) {
                    DataPiece piece;
                    piece.buf = buf;
                    piece.off = off;
                    piece.len = len;
                    piece.bytesSoFar = bytesSoFar;
                    handler(connId, piece);
                }
                if (len <= 0) {
                    onStreamFinished(connId);
                }
            });

        std::unique_lock<std::mutex> lock(m_socketsMutex);
        m_socketPullers[connId] = {puller, false};
        lock.unlock();

        if (newConnectionHandler) {
            newConnectionHandler(sock);
        }

        puller->start();  // Start the puller after adding it to the puller list.
    }

    // Called in individual puller thread.
    void onStreamFinished(uint32_t connId) {
        AACE_INFO(LX(TAG).d("connId", connId));

        std::unique_lock<std::mutex> lock(m_socketsMutex);
        auto it = m_socketPullers.find(connId);
        if (it != m_socketPullers.end()) {
            // We cannot shutdown a puller here since this method is running in puller thread.
            it->second.finished = true;
        }
    }

    void shutdownPullers(bool force) {
        AACE_INFO(LX(TAG).d("force", force));

        if (force) {
            std::unique_lock<std::mutex> lock(m_socketsMutex);
            // Make a copy to prevent dead lock with onStreamFinished
            decltype(m_socketPullers) pullers = m_socketPullers;
            lock.unlock();

            for (auto it = pullers.begin(); it != pullers.end(); ++it) {
                auto& info = it->second;
                AACE_INFO(LX(TAG).m("Shutdown puller").d("connId", it->first));
                info.puller->shutdown();
            }

            lock.lock();
            m_socketPullers.clear();
        } else {
            std::unique_lock<std::mutex> lock(m_socketsMutex);
            for (auto it = m_socketPullers.begin(); it != m_socketPullers.end();) {
                auto& info = it->second;
                if (info.finished) {
                    AACE_INFO(LX(TAG).m("Shutdown puller").d("connId", it->first));
                    info.puller->shutdown();
                    it = m_socketPullers.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    void sendResponse(uint32_t connId, uint8_t* buf, int off, int len) {
        std::unique_lock<std::mutex> lock(m_socketsMutex);
        auto it = m_socketPullers.find(connId);
        if (it != m_socketPullers.end()) {
            auto puller = it->second.puller;
            lock.unlock();

            if (buf == nullptr) {  // The receiver would like to end the connection
                puller->close();
                return;
            }
            puller->sendResponse(buf, off, len);
        }
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
static const char* TAG = "TcpProxy";

TcpProxy::TcpProxy(int port, DataHandler dataHandler, NewConnectionHandler newConnectionHandler) {
    AACE_INFO(LX(TAG).d("port", port));
    m_impl = std::make_unique<Impl>(port, dataHandler, newConnectionHandler);
}

TcpProxy::~TcpProxy() {
    shutdown();
}

void TcpProxy::sendResponse(uint32_t connId, uint8_t* buf, int off, int len) {
    m_impl->sendResponse(connId, buf, off, len);
}

void TcpProxy::shutdown() {
    m_impl->shutdown();
}

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace
